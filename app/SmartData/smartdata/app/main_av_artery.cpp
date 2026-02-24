#define AV_PROJECT
#define ARTERY_PROJECT
#define CARLA_V2_PROJECT
#define AV_MODULES_PYTHON

#include <main_traits.h>
#include <network/tstp/tstp.h>
#include <architecture/cpu.h>
#include <transducer.h>
#include <transformer.h>
#include <smartdata.h>
#include <boolean_filters.h>
#include <utility/geometry.h>

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define Delay usleep

OStream cout;

static const int LAST_DEVICE = 44;
static const UInt32 EXPIRY = 100000;

List<pid_t> *daemons;
UDP_Socket *sdhandler_socket;
pid_t *my_pid;

SEU_SmartData* seu_sd;
Unit_Dev_Expiry::List* ud_list;
Monitoring* monitor;
bool ended;

typedef IF<Traits<Project>::CAM_IS_TRANSFORMER, Object_Recognition_And_Tracking_Camera_Transformer, Object_Recognition_And_Tracking_Camera>::Result Camera_ORT;
typedef IF<Traits<Project>::LIDAR_IS_TRANSFORMER, Object_Recognition_And_Tracking_LIDAR_Transformer, Object_Recognition_And_Tracking_LIDAR>::Result LIDAR_ORT;

// Declaratoin of all interests types that would be used explicitly in main
using Brake_Source_Proxy =      Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Acceleration | SmartData::Unit::F32)>>;
using Steer_Source_Proxy =      Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Angle | SmartData::Unit::F32)>>;
using Reverse_Source_Proxy =    Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Counter)>>;
using Hand_Brake_Source_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Counter)>>;
using Throttle_Source_Proxy =  Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Acceleration | SmartData::Unit::F32)>>;

void Usage();

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations);

void clean_before_exit() {
    pid_t *d;
    while (!daemons->empty()) {
        d = daemons->remove_tail()->object(); // it is not necessary to send the kill signal, this will be done by sink...
        delete d;
    }
    delete daemons;
    delete sdhandler_socket;
    if (my_pid != 0)
        delete my_pid;

    if (seu_sd != 0) {
        delete seu_sd;
        delete ud_list;
        delete monitor;
    }

    TSTP::finish();
    Thread::finish();
}

void sig_handler(int sig) {
    switch (sig) {
    case SIGKILL:
    case SIGTERM:
        cout << "SINK called kill... --> my_pid=" << getpid() << endl;
        clean_before_exit();
        //exit(0);
    default:
        cout << stderr << "... wasn't expecting that!" << endl;
        //abort();
    }
    ended = true;
}

void seu() {
    int fd;
    char log_file[24] = "logs/sniffer.log";
    fd = open(log_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    dup2(fd, STDOUT_FILENO);

    cout << "TSTP::init()...";
    TSTP::init();
    cout << "done." << endl;
    seu_sd = new SEU_SmartData(0);
    cout << "SEU:" << seu_sd << endl;

    ud_list = new Unit_Dev_Expiry::List(); // List do not have desctructors, we need to delete here or to implement it on lists....

    ud_list->insert((new Unit_Dev_Expiry(Dynamics_State::UNIT, 16, EXPIRY))->link());
    ud_list->insert((new Unit_Dev_Expiry(ETSI_CAM_Source::UNIT, 26, EXPIRY))->link());
    ud_list->insert((new Unit_Dev_Expiry(ETSI_CPM_Source::UNIT, 28, EXPIRY))->link());
    ud_list->insert((new Unit_Dev_Expiry(Fuel_Consumption::UNIT, 42, EXPIRY))->link());
    ud_list->insert((new Unit_Dev_Expiry(CO2_Emission::UNIT, 43, EXPIRY))->link());

    cout << "SEU UD_List done!" << endl;

    monitor = new Monitoring(ud_list);
    cout << "Monitor created!" << endl;

    seu_sd->add_boolean_filter(monitor);

    cout << "Waiting for data!\nLog Start:" << endl;
    while(1) { if (ended) break; Delay(15*1000000); }
}

class Interest_To_Trigger_Monitoring : public Observer
{
    using Dynamics_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::MOTION_VECTOR_LOCAL|12<<16|1)>>;
    using CAM_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::MOTION_VECTOR_GLOBAL | 1)>>;
    using CPM_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::MOTION_VECTOR_GLOBAL | 30)>>;
    using Fuel_Consumption_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Mass_Flow | SmartData::Unit::D64)>>;
    using CO2_Emission_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Mass_Flow | SmartData::Unit::D64)>>;

    public:
        Interest_To_Trigger_Monitoring() {
            d_p = new Dynamics_Proxy(CAM_Proxy::Region(0, 0, 0, 100, CAM_Proxy::now(),    INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 16);
            cam_p = new CAM_Proxy(CAM_Proxy::Region(0, 0, 0, 100, CAM_Proxy::now(),    INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 26);
            cpm_p = new CPM_Proxy(CPM_Proxy::Region(0, 0, 0, 100, CPM_Proxy::now(),    INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 28);
            fc_p = new Fuel_Consumption_Proxy(Fuel_Consumption_Proxy::Region(0, 0, 0, 100, Fuel_Consumption_Proxy::now(),    INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 42);
            co2_p = new CO2_Emission_Proxy(CO2_Emission_Proxy::Region(0, 0, 0, 100, CO2_Emission_Proxy::now(),    INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 43);

            d_p->attach(this);
            cam_p->attach(this);
            cpm_p->attach(this);
            fc_p->attach(this);
            co2_p->attach(this);
        }

        virtual ~Interest_To_Trigger_Monitoring() {
            delete d_p;
            delete cam_p;
            delete cpm_p;
            delete fc_p;
            delete co2_p;
        }

        void update(Observed *obs) {
            cout << "New data arrived... Check sniffer to see data... \n";
        }

    private:
        Dynamics_Proxy * d_p;
        CAM_Proxy * cam_p;
        CPM_Proxy * cpm_p;
        Fuel_Consumption_Proxy *fc_p;
        CO2_Emission_Proxy *co2_p;
};

int main(int argc, char* argv[])
{
    cout << "Starting system abstractions..." << endl;
    cout << "Thread::init()...";
	Thread::init();
    cout << "done." << endl;
    daemons = new List<pid_t>();

    if (argc == 2 && argv[1][1] == 'e') { // "seu"
        signal(SIGKILL, sig_handler);
        cout << "Creating SEU: ";
        my_pid = new pid_t(fork());
        if (*my_pid == 0)
            seu(); // seu never leaves this function until a kill signal is sent --> handler deletes daemons pointer
        else {
            cout << "SEU PID=" << *my_pid << endl;
            daemons->insert(new List<pid_t>::Element(my_pid));
        }
    } else if (argc > 2) {
        cout << "Wrong usage..." << endl;
		Usage();
		return -1;
    }
    my_pid = new pid_t(fork());
    if (*my_pid == 0) {
        cout << "Creating Interest_To_Trigger_Monitoring...";
        Interest_To_Trigger_Monitoring *i = new Interest_To_Trigger_Monitoring();
        cout << "Done!" << endl;
        while(true) { if (ended) break; Delay(EXPIRY); };
        cout << "Deleting Interest_To_Trigger_Monitoring...";
        delete i;
        cout << "Done!" << endl;

    } else {
        cout << "SmartDataHandler is up and waiting for commands..." << endl;
        sdhandler_socket = new UDP_Socket(-1, false);
    }
    UInt32 dev;
    UInt32 period;
    UInt32 iterations;
    UInt32 expiry;

    const unsigned char *data;

    cout << "TSTP::init()...";
    TSTP::init();
    cout << "done." << endl;

    int count = 0;

    while(true) {
        data = sdhandler_socket->receive(); // allways a 1B+4*4B --> 'q' or 'c' followed by 4 uint32
        cout << "I've read changes=" << data << endl;

        if (data[0] == 'q') {
            break;
        } else {
            dev = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[1]));
            period = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[5]));
            expiry = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[9]));
            iterations = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[13]));
            cout << "Received: dev="<< dev << ",p=" << period << ",e=" << expiry << ",i=" << iterations << endl;
            if (dev > LAST_DEVICE) {
                cout << dev << " unsupported_device!" << endl;
                continue;
            }
            my_pid = new pid_t(fork()); // maybe change this to thread later
            if (*my_pid == 0) {
                if (Traits<Build>::debugged) {
                    // as this is on child process, it will only run once, so it is safe to declare variables here
                    int fd;
                    char log_file[24] = "logs/";
                    char buff[12];
                    char log_file_suff[5] = ".log";
                    sprintf(buff, "%u", dev);
                    strcat(log_file,buff);
                    strcat(log_file,log_file_suff);

                    fd = open(log_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);

                    dup2(fd, STDOUT_FILENO);
                } else {
                    close(1); // close stdout
                }
                cout << "TSTP::init()...";
                TSTP::init();
                cout << "done." << endl;
                create_daemon(dev, period, expiry, iterations); // nodes are allways in their while (true) until a kill signal is sent
                return 0;
            } else if (*my_pid < 0) {
                cout << dev << "Error on creation!" << *my_pid << endl;
            } else {
                daemons->insert(new List<pid_t>::Element(my_pid));
                cout << dev << " created, pid=" << *my_pid << endl;
                count++;
                if (count == 5) {
                    my_pid = new pid_t(fork());
                    if (*my_pid == 0) {
                        // as this is on child process, it will only run once, so it is safe to declare variables here
                        int fd;
                        char log_file[24] = "logs/";
                        char log_file_suff[5] = ".log";
                        strcat(log_file,"trigger");
                        strcat(log_file,log_file_suff);

                        fd = open(log_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);

                        dup2(fd, STDOUT_FILENO);
                        cout << "Creating Interest_To_Trigger_Monitoring...";
                        Delay(EXPIRY);
                        Interest_To_Trigger_Monitoring *i = new Interest_To_Trigger_Monitoring();
                        cout << "Done!" << endl;
                        while(true) { if (ended) break; Delay(EXPIRY); };
                        cout << "Deleting Interest_To_Trigger_Monitoring...";
                        delete i;
                        cout << "Done!" << endl;
                    }
                }
            }
        }
    }

    pid_t *d;
    int status;
    while (!daemons->empty()) {
        d = daemons->remove_tail()->object();
        cout << "killing pid=" << *d;
        kill(*d, SIGKILL);
        waitpid(*d, &status, 0);
        cout << ", status=" << status << endl;
        delete d;
    }
    delete sdhandler_socket; // only sink initializes this pointer

    delete daemons;

    Thread::finish();

    return 0;
}

void Usage()
{
    cout << "Usage: (create only sink)" << endl;
    cout << "smartdata" << endl;
    cout << "  OR (create sink and seu)" << endl;
    cout << "smartdata seu" << endl;
}

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations)
{
    switch (dev)
    {
    case 0: {
        cout << "I'm a longitudinal acceleration (Sensor w/dev=0) -- Not supported";
        break;
    }
    case 1: {
        cout << "I'm a lateral acceleration (Sensor w/dev=1) -- Not supported";
        break;
    }
    case 2: {
        cout << "I'm a vertical acceleration (Sensor w/dev=2) -- Not supported";
        break;
    }
    case 36: {
        cout << "I'm a yaw rate (Sensor w/dev=36) -- Not supported";
        break;
    }
    case 37: {
        cout << "I'm a pitch rate (Sensor w/dev=37) -- Not supported";
        break;
    }
    case 38: {
        cout << "I'm a roll rate (Sensor w/dev=38) -- not supported";
        break;
    }
    case 3: {
        cout << "Speed X (Sensor w/dev=3) -- not supported";
        break;
    }
    case 4: {
        cout << "Speed Y (Sensor w/dev=4) -- not supported";
        break;
    }
    case 5: {
        cout << "Speed Z (Sensor w/dev=5) -- not supported";
        break;
    }
    case 6: {
        break;
    }
    case 7: {
        cout << "I'm a longitude (Sensor w/dev=7) -- Not supported";
        break;
    }
    case 8: {
        cout << "I'm a latitude (Sensor w/dev=8) -- Not supported";
        break;
    }
    case 9: {
        cout << "I'm an altitude (Sensor w/dev=9) -- Not supported";
        break;
    }
    case 10: {
        cout << "Gear (Sensor w/dev=10)... -- not supported";
        break;
    }
    case 11: {
        cout << "Engine RPM (Sensor w/dev=11)...  -- not supported";
        break;
    }
    case 12: {
        cout << "Battery State of Charge (Sensor w/dev=12)... -- not supported";
        break;
    }
    case 13: {
        cout << "I'm a yaw (Sensor w/dev=13) -- Not supported";
        break;
    }
    case 14: {
        cout << "I'm a pitch (Sensor w/dev=14) -- Not supported";
        break;
    }
    case 15: {
        cout << "I'm a roll (Sensor w/dev=15) -- Not supported";
        break;
    }
    case 41: {
        cout << "Destination (Transformer w/dev=41) -- Not supported";
        break;
    }
    case 16: {
        cout << "Dynamics_State (Transformer w/dev=16) -- Kalman Filters...";
        Dynamics_State dyn(16, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {if (ended) break; Delay(period);}
        break;
    }
    case 17: {
        cout << "Camera Image (Sensor w/dev=17) -- Not supported";
        break;
    }
    case 18: {
        cout << "LIDAR Cloud Point (Sensor w/dev=18) -- Not supported";
        break;
    }
    case 19: {
        cout << "RADAR Cloud Point (Sensor w/dev=19) -- Not supported";
        break;
    }
    case 20: {
        cout << "CAMERA Object Recognition and Tracking (dev=20) -- Not supported";
        break;
    }
    case 21: {
        cout << "LIDAR Object Recognition and Tracking (dev=21) -- Not supported";
        break;
    }
    case 22: {
        cout << "RADAR Object Recognition and Tracking (Transformer w/dev=22) -- Not supported";
        break;
    }
    case 23: {
        cout << "Fuser Object Recognition and Tracking (Transformer w/dev=23) -- Not supported";
        break;
    }
    case 24: {
        cout << "Map (Sensor w/dev=24) -- Not supported";
        break;
    }
    case 25: {
        cout << "Map Orientation (Sensor w/dev=25) -- Not supported";
        break;
    }
    case 26: {
        cout << "ETSI-CAM Messages (Sensor) (Transducer w/dev=26)...";
        ETSI_CAM_Source f(26, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {if (ended) break; Delay(period);}
        break;
    }
    case 27: {
        cout << "ETSI-DENM Messages (Sensor) not implement yet";
        break;
    }
    case 28: {
        cout << "ETSI-CPM Messages (Sensor) (Transducer w/dev=28)...";
        ETSI_CPM_Source f(28, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {if (ended) break; Delay(period);}
        break;
    }
    case 29: {
        cout << "Path Planning (Transformerw/dev=29) -- Not supported";
        break;
    }
    case 30: {
        cout << "Motion Planning (Transformerw/dev=30) -- Not supported";
        break;
    }
    case 31: {
        cout << "Steering (Actuator)  -- Not supported";
        break;
    }
    case 32: {
        cout << "Throttle (Actuator) -- Not supported";
        break;
    }
    case 33: {
        cout << "Brake (Actuator) -- Not supported";
        break;
    }
    case 34: {
        cout << "Reverse (Actuator) -- Not supported";
        break;
    }
    case 35: {
        cout << "Hand Brake (Actuator) -- Not supported";
        break;
    }
    case 39: {
        cout << "High-Level Control (Transformer) -- Not supported";
        break;
    }
    case 42: {
        cout << "Fuel Consumption (Sensor) (Transducer w/dev=42)...";
        Fuel_Consumption f(42, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {if (ended) break; Delay(period);}
        break;
    }
    case 43: {
        cout << "CO2 Emission (Sensor) (Transducer w/dev=43)...";
        CO2_Emission c(43, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {if (ended) break; Delay(period);}
        break;
    }
    default:
        cout << "Invalid Device, waiting for kill signal!" << endl;
        break;
    }
    cout << "Good bye!" << endl;
}

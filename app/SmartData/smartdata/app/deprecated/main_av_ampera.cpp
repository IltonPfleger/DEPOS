#define AV_PROJECT

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

static const int LAST_DEVICE = 39;

List<pid_t> *daemons;
UDP_Socket *sdhandler_socket;
pid_t *my_pid;

SEU_SmartData* seu_sd;
Unit_Dev_Expiry::List* ud_list;
Monitoring* monitor;

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
        exit(0);
    default:
        cout << stderr << "... wasn't expecting that!" << endl;
        abort();
    }
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
    // IMU XYZ Acceleration
    ud_list->insert((new Unit_Dev_Expiry(Longitudinal_Acceleration::UNIT, 0, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Lateral_Acceleration::UNIT, 1, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Vertical_Acceleration::UNIT, 2, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Yaw_Rate::UNIT, 36, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Pitch_Rate::UNIT, 37, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Roll_Rate::UNIT, 38, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Speed::UNIT, 3, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Speed::UNIT, 4, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Speed::UNIT, 5, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Longitude::UNIT, 7, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Latitude::UNIT, 8, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Altitude::UNIT, 9, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Gear::UNIT, 10, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Engine_RPM::UNIT, 11, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Battery_Charge::UNIT, 12, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Yaw::UNIT, 13, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Pitch::UNIT, 14, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Roll::UNIT, 15, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Dynamics_State::UNIT, 16, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Camera_AV::UNIT, 17, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LIDAR_AV::UNIT, 18, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(RADAR_AV_AMPERA::UNIT, 19, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_Camera_Transformer::UNIT, 20, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_LIDAR::UNIT, 21, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_RADAR::UNIT, 22, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_Fuser::UNIT, 23, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Map::UNIT, 24, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Path_Planning::UNIT, 29, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Steer_Actuator::UNIT, 31, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Throttle_Actuator::UNIT, 32, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Brake_Actuator::UNIT, 33, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Reverse_Actuator::UNIT, 34, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Hand_Brake_Actuator::UNIT, 35, 100000))->link());
    cout << "SEU UD_List done!" << endl;

    monitor = new Monitoring(ud_list);
    cout << "Monitor created!" << endl;

    seu_sd->add_boolean_filter(monitor);

    cout << "Waiting for data!\nLog Start:" << endl;
    while(1) { Delay(15*1000000); }
}

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

    cout << "SmartDataHandler is up and waiting for commands..." << endl;
    sdhandler_socket = new UDP_Socket(-1, false);
    UInt32 dev;
    UInt32 period;
    UInt32 iterations;
    UInt32 expiry;

    const unsigned char *data;

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
                if (Traits<Debug>::trace || Traits<Debug>::warning) {
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
            } else if (*my_pid < 0) {
                cout << dev << "Error on creation!" << *my_pid << endl;
            } else {
                daemons->insert(new List<pid_t>::Element(my_pid));
                cout << dev << " created, pid=" << *my_pid << endl;
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
        cout << "I'm a longitudinal acceleration (Sensor w/dev=0)";
        Longitudinal_Acceleration longitudinal_acceleration(0, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 1: {
        cout << "I'm a lateral acceleration (Sensor w/dev=1)";
        Lateral_Acceleration lateral_acceleration(1, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 2: {
        cout << "I'm a vertical acceleration (Sensor w/dev=2)";
        Vertical_Acceleration vertical_acceleration(2, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 36: {
        cout << "I'm a yaw rate (Sensor w/dev=36)";
        Yaw_Rate yaw_rate(36, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 37: {
        cout << "I'm a pitch rate (Sensor w/dev=37)";
        Pitch_Rate pitch_rate(37, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 38: {
        cout << "I'm a roll rate (Sensor w/dev=38)";
        Roll_Rate roll_rate(38, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 3: {
        cout << "Speed X (Sensor w/dev=3)";
        Speed s(3, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 4: {
        cout << "Speed Y (Sensor w/dev=4)";
        Speed s(4, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 5: {
        cout << "Speed Z (Sensor w/dev=5)";
        Speed s(5, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 6: {
        break;
    }
    case 7: {
        cout << "I'm a longitude (Sensor w/dev=7)";
        Longitude longitude(7, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 8: {
        cout << "I'm a latitude (Sensor w/dev=8)";
        Latitude latitude(8, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 9: {
        cout << "I'm an elevation (Sensor w/dev=9)";
        Altitude elevation(9, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 10: {
        cout << "Gear (Sensor w/dev=10)...";
        Gear g(10, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 11: {
        cout << "Engine RPM (Sensor w/dev=11)...";
        Engine_RPM rpm(11, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 12: {
        cout << "Battery State of Charge (Sensor w/dev=12)...";
        Battery_Charge bc(12, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 13: {
        cout << "I'm a yaw (Sensor w/dev=13)";
        Yaw yaw(13, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 14: {
        cout << "I'm a pitch (Sensor w/dev=14)";
        Pitch pitch(14, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 15: {
        cout << "I'm a roll (Sensor w/dev=15)";
        Roll roll(15, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 16: {
        cout << "Dynamics_State (Transformer w/dev=16) -- Kalman Filters...";
        Dynamics_State dyn(16, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 17: {
        cout << "Camera Image (Sensor w/dev=17)...";
        Camera_AV c(17, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 18: {
        cout << "LIDAR Cloud Point (Sensor w/dev=18)...";
        LIDAR_AV l(18, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 19: {
        cout << "RADAR Cloud Point (Sensor w/dev=19)...";
        RADAR_AV r(19, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
    }
    case 20: {
        cout << "CAMERA Object Recognition and Tracking (Sensor/Transformer w/dev=20)...";
        Object_Recognition_And_Tracking_Camera_Transformer c(20, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 21: {
        cout << "LIDAR Object Recognition and Tracking (Transformer w/dev=21)...";
        Object_Recognition_And_Tracking_LIDAR c(21, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 22: {
        cout << "RADAR Object Recognition and Tracking (Transformer w/dev=22)...";
        Object_Recognition_And_Tracking_RADAR c(22, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 23: {
        cout << "Fuser Object Recognition and Tracking (Transformer w/dev=23)...";
        Object_Recognition_And_Tracking_Fuser c(23, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 24: {
        cout << "Map (Sensor w/dev=24)...";
        Map m(24, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 25: {
        cout << "Map Orientation is not anymore a sensor in SDAV -- orientation is always in relation to north of WGS84 (Sensor w/dev=25)...";
        break;
    }
    case 26: {
        cout << "ETSI-CAM Messages (Sensor) not implement yet";
        break;
    }
    case 27: {
        cout << "ETSI-DENM Messages (Sensor) not implement yet";
        break;
    }
    case 28: {
        cout << "ETSI-CPM Messages (Sensor) not implement yet";
        break;
    }
    case 29: {
        cout << "Path Planning (Transformerw/dev=29)...";
        Path_Planning p(29, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 30: {
        cout << "Motion Planning (Transformerw/dev=30)...";
        Motion_Planning c(30, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 31: {
        cout << "Steering (Actuator)";
        Steer_Source_Proxy sp(Steer_Source_Proxy::Region(0, 0, 0, 100, Steer_Source_Proxy::now(), Steer_Source_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 31);
        Steer_Actuator sa(31, expiry);
        sp.attach(&sa);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 32: {
        cout << "Throttle (Actuator)";
        Throttle_Source_Proxy tp(Throttle_Source_Proxy::Region(0, 0, 0, 100, Throttle_Source_Proxy::now(), Throttle_Source_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 32);
        Throttle_Actuator ta(32, expiry);
        tp.attach(&ta);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 33: {
        cout << "Brake (Actuator)";
        Brake_Source_Proxy bp(Brake_Source_Proxy::Region(0, 0, 0, 100, Brake_Source_Proxy::now(), Brake_Source_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 33);
        Brake_Actuator ba(33, expiry);
        bp.attach(&ba);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 34: {
        cout << "Reverse (Actuator)";
        Reverse_Source_Proxy rp(Reverse_Source_Proxy::Region(0, 0, 0, 100, Reverse_Source_Proxy::now(), Reverse_Source_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 34);
        Reverse_Actuator ra(34, expiry);
        rp.attach(&ra);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 35: {
        cout << "Hand Brake (Actuator)";
        Hand_Brake_Source_Proxy hp(Hand_Brake_Source_Proxy::Region(0, 0, 0, 100, Hand_Brake_Source_Proxy::now(), Hand_Brake_Source_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 35);
        Hand_Brake_Actuator ha(35, expiry);
        hp.attach(&ha);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 39: {
        cout << "High-Level Control (Transformer)";
        High_Level_Control c(39, expiry);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    default:
        cout << "Invalid Device, waiting for kill signal!" << endl;
        break;
    }
    cout << "Good bye!" << endl;
}

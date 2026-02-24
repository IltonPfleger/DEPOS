#define AV_PROJECT
#define CARLA_V2_PROJECT
#define AV_MODULES_PYTHON

#include <architecture/cpu.h>
#include <arpa/inet.h>
#include <boolean_filters.h>
#include <fcntl.h>
#include <main_traits.h>
#include <netinet/in.h>
#include <network/tstp/tstp.h>
#include <signal.h>
#include <smartdata.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <transducer.h>
#include <transformer.h>
#include <unistd.h>
#include <utility/geometry.h>

#define Delay usleep

using Dynamics_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(Dynamics_State_KF_On_Hardware_SD::UNIT)>>;

OStream cout;

static const int LAST_DEVICE = 49;

List<pid_t>* daemons;
pid_t* my_pid;

SEU_SmartData* seu_sd;
Unit_Dev_Expiry::List* ud_list;
Monitoring* monitor;
Dynamics_Proxy* dynamics;

bool ended;
void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations);

void clean_before_exit()
{
    pid_t* d;
    while (!daemons->empty()) {
        d = daemons->remove_tail()->object();  // it is not necessary to send the kill signal, this will be done by sink...
        delete d;
    }
    delete daemons;
    if (my_pid != 0) delete my_pid;

    if (seu_sd != 0) {
        delete seu_sd;
        delete ud_list;
        delete monitor;
    }

    TSTP::finish();
    Thread::finish();
}

void sig_handler(int sig)
{
    switch (sig) {
        case SIGKILL:
        case SIGTERM:
            cout << "SINK called kill... --> my_pid=" << getpid() << endl;
            clean_before_exit();
            // exit(0);
        default:
            cout << stderr << "... wasn't expecting that!" << endl;
            // abort();
    }
    ended = true;
}

void seu()
{
    // int fd;
    // char log_file[24] = "logs/sniffer.log";
    // fd                = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    //  dup2(fd, STDOUT_FILENO);

    cout << "TSTP::init()...";
    TSTP::init();
    cout << "done." << endl;

    seu_sd  = new SEU_SmartData(0);
    ud_list = new Unit_Dev_Expiry::List();
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Longitudinal_Acceleration::UNIT, 0, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Lateral_Acceleration::UNIT, 1, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Vertical_Acceleration::UNIT, 2, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_GNSS_Longitude::UNIT, 7, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_GNSS_Latitude::UNIT, 8, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_GNSS_Altitude::UNIT, 9, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_GNSS_Velocity::UNIT, 3, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Yaw::UNIT, 13, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Yaw_Rate::UNIT, 36, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Pitch_Rate::UNIT, 37, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LPMS_IMU_Roll_Rate::UNIT, 38, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Dynamics_State_KF_On_Hardware_SD::UNIT, 16, 100000))->link());

    monitor = new Monitoring(ud_list);
    seu_sd->add_boolean_filter(monitor);

    cout << "Waiting for data!\nLog Start:" << endl;
    while (1) {
        if (ended) break;
        Delay(15 * 1000000);
    }
}

int create_forked(UInt32 dev, UInt32 period, UInt32 iterations, UInt32 expiry)
{
    cout << "Received: dev=" << dev << ",p=" << period << ",e=" << expiry << ",i=" << iterations << endl;
    if (dev > LAST_DEVICE) {
        cout << dev << " unsupported_device!" << endl;
        return -1;
    }
    my_pid = new pid_t(fork());  // maybe change this to thread later
    if (*my_pid == 0) {
        if (Traits<Build>::debugged) {
            // as this is on child process, it will only run once, so it is safe to declare variables here
            int fd;
            char log_file[24] = "logs/";
            char buff[12];
            char log_file_suff[5] = ".log";
            sprintf(buff, "%u", dev);
            strcat(log_file, buff);
            strcat(log_file, log_file_suff);

            fd = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

            dup2(fd, STDOUT_FILENO);
        } else {
            close(1);  // close stdout
        }
        cout << "TSTP::init()...";
        TSTP::init();
        cout << "done." << endl;
        create_daemon(dev, period, expiry, iterations);  // nodes are allways in their while (true) until a kill signal is sent
        return 0;
    } else if (*my_pid < 0) {
        cout << dev << "Error on creation!" << *my_pid << endl;
    } else {
        daemons->insert(new List<pid_t>::Element(my_pid));
        cout << dev << " created, pid=" << *my_pid << endl;
    }
    return 1;
}

int main(int argc, char* argv[])
{
    cout << "Thread::init()...";
    Thread::init();
    cout << "done." << endl;
    cout << "TSTP::init()...";
    TSTP::init();
    cout << "done." << endl;

    daemons = new List<pid_t>();

    if (argc == 2 && argv[1][1] == 'e') {  // "seu"
        signal(SIGKILL, sig_handler);
        cout << "Creating SEU: ";
        my_pid = new pid_t(fork());
        if (*my_pid == 0)
            seu();  // seu never leaves this function until a kill signal is sent --> handler deletes daemons pointer
        else {
            cout << "SEU PID=" << *my_pid << endl;
            daemons->insert(new List<pid_t>::Element(my_pid));
        }
    }

    UInt32 dev;
    UInt32 period     = 100000;
    UInt32 iterations = 100000;
    UInt32 expiry     = 100000;
    int ret                 = 0;

    for (dev = 0; dev < LAST_DEVICE; dev++) {
        ret = create_forked(dev, period, iterations, expiry);
        if (ret != 1) {
            break;
        }
    }

    dynamics = new Dynamics_Proxy(Dynamics_Proxy::Region(0, 0, 0, 100, Dynamics_Proxy::now(), INFINITE), expiry, period, SmartData::SINGLE, SmartData::ANY, 16);

    while (true) {
        Delay(period * 10);
    }

    pid_t* d;
    int status;
    while (!daemons->empty()) {
        d = daemons->remove_tail()->object();
        cout << "killing pid=" << *d;
        kill(*d, SIGKILL);
        waitpid(*d, &status, 0);
        cout << ", status=" << status << endl;
        delete d;
    }

    delete daemons;

    Thread::finish();

    return 0;
}

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations)
{
    switch (dev) {
        case 0: {
            cout << "I'm a longitudinal acceleration (Sensor w/dev=0)";
            LPMS_IMU_Longitudinal_Acceleration longitudinal_acceleration(0, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a lateral acceleration (Sensor w/dev=1)";
            LPMS_IMU_Lateral_Acceleration lateral_acceleration(1, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a vertical acceleration (Sensor w/dev=2)";
            LPMS_IMU_Vertical_Acceleration vertical_acceleration(2, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a yaw rate (Sensor w/dev=36)";
            LPMS_IMU_Yaw_Rate yaw_rate(36, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a pitch rate (Sensor w/dev=37)";
            LPMS_IMU_Pitch_Rate pitch_rate(37, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a roll rate (Sensor w/dev=38)";
            LPMS_IMU_Roll_Rate roll_rate(38, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a yaw (Sensor w/dev=12)";
            LPMS_IMU_Yaw yaw(13, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a longitude (Sensor w/dev=7)";
            LPMS_GNSS_Longitude longitude(7, 1000000, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a latitude (Sensor w/dev=8)";
            LPMS_GNSS_Latitude latitude(8, 1000000, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm an altitude (Sensor w/dev=9)";
            LPMS_GNSS_Altitude altitude(9, 1000000, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            cout << "I'm a Speed X (Sensor w/dev=10)";
            LPMS_GNSS_Velocity s(3, 1000000, SmartData::ADVERTISED);
            cout << "Done!" << endl;

            while (true) {
                Delay(period);
            }
            break;
        }
        case 16: {
            cout << "Dynamics_State (Transformer w/dev=16) -- Kalman Filters...";
            Dynamics_State_KF_On_Hardware_SD dyn(16, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        default: {
            cout << "Invalid Device, waiting for kill signal!" << endl;
            while (true) {
                Delay(period);
            }
            break;
        }
    }
    cout << "Good bye!" << endl;
}

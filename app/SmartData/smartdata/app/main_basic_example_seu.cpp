#define BASIC_EXAMPLE

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

static const int LAST_DEVICE = 3;

List<pid_t> *daemons;
UDP_Socket *sdhandler_socket;
pid_t *my_pid;

SEU_SmartData* seu_sd;
Unit_Dev_Expiry::List* ud_list;
Monitoring* monitor;
Environment_Min_Max_Temperature_Boolean_Filter* temperature_bf;

// Declaratoin of all interests types that would be used explicitly in main
using Temperature_Adjust_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Switch)>>;

void Usage();

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations);
void seu();

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
    if (seu_sd != 0)
        delete seu_sd;
    if (ud_list != 0)
        delete ud_list;
    if (monitor != 0)
        delete monitor;
    if (temperature_bf != 0)
        delete temperature_bf;

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
        data = sdhandler_socket->receive(); // allways a 1B+4B --> 'q' or 'c' followed by 1 uint32
        cout << "I've read changes=" << data << endl;

        if (data[0] == 'q') {
            break;
        } else {
            dev = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[1]));
            period = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[5]));
            expiry = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[9]));
            iterations = *reinterpret_cast<UInt32 *>(const_cast<unsigned char *>(&data[13]));
            if (dev > LAST_DEVICE) {
                cout << dev << " unsupported_device!" << endl;
                continue;
            }
            cout << "creating Daemon: dev=" << dev << ",period=" << period << ",expiry=" << expiry << endl;
            my_pid = new pid_t(fork()); // maybe change this to thread later
            if (*my_pid == 0) {
                close(1); // close stdout
                cout << "TSTP::init()...";
                TSTP::init();
                cout << "done." << endl; 
                create_daemon(dev, period, expiry, iterations); // nodes are allways in their while (true) until a kill signal is sent
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
        kill(*d, SIGKILL);
        waitpid(*d, &status, 0);
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

void seu() {

    int fd;
    char log_file[24] = "logs/sniffer.log";
    fd = open(log_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    cout << "TSTP::init()...";
    TSTP::init();
    cout << "done." << endl; 

    ud_list = new Unit_Dev_Expiry::List(); // List do not have desctructors, we need to delete here or to implement it on lists....
    // IMU XYZ Acceleration
    ud_list->insert((new Unit_Dev_Expiry(Temperature::UNIT, 0, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Temperature::UNIT, 1, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Temperature_Adjust::UNIT, 2, 100000))->link());
    cout << "SEU UD_List done!" << endl;

    monitor = new Monitoring(ud_list, 100000);
    ud_list = new Unit_Dev_Expiry::List();
    ud_list->insert((new Unit_Dev_Expiry(Temperature::UNIT, 0, 100000))->link());
    temperature_bf = new Environment_Min_Max_Temperature_Boolean_Filter(ud_list, 0, 100000);

    cout << "Monitor created!" << endl;
    seu_sd = new SEU_SmartData(0);
    cout << "SEU:" << seu_sd << endl;
    seu_sd->add_boolean_filter(temperature_bf);
    seu_sd->add_boolean_filter(monitor);

    cout << "Waiting for data!\nLog Start:" << endl;
    while(1) { Delay(15*1000000); }

}

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations)
{
    switch (dev)
    {
    case 0: {
        cout << "Temperature sensor...";
        Temperature t(0, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 1: {
        cout << "Ideal Temperature (Sensor)";
        Temperature it(1, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 2: {
        cout << "Temperature Adjust (Transfomer)";
        Temperature_Adjust s(2, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 3: {
        cout << "Cooler Controller (Actuator)";
        Temperature_Adjust_Proxy tap(Temperature_Adjust_Proxy::Region(0, 0, 0, 100, Temperature_Adjust_Proxy::now(), Temperature_Adjust_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 2);
        Cooler_Actuator ca(3, expiry);
        tap.attach(&ca);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    default:
        cout << "Invalid Device, waiting for kill signal!" << endl;
        break;
    }
    cout << "Good bye!" << endl;
    while(true) {cout << "waiting for kill signal!" << endl;}
}

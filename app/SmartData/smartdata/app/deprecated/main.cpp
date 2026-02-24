#define CARLA_V1_PROJECT

#include <main_traits.h>
#include <network/tstp/tstp.h>
#include <architecture/cpu.h>
#include <transducer.h>
#include <transformer.h>
#include <smartdata.h>
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

#define Delay usleep

OStream cout;

static const int LAST_DEVICE = 12;

List<pid_t> *daemons;
UDP_Socket *sdhandler_socket;
pid_t *my_pid;

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
    cout << "SEU is unsupported in this version" << endl;
}

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations)
{
    switch (dev)
    {
    case 0: {
        cout << "Camera (Sensor w/dev=0)...";
        Camera_CARLA c(0, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 1: {
        cout << "I'm a Direction (Sensor)";
        Direction d(1, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 2: {
        cout << "I'm a Forward Speed (Sensor)";
        Speed s(2, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 3: {
        cout << "I'm a Steer Angle (Actuator)";
        Angle_Remote_Proxy angp(Angle_Remote::Region(0, 0, 0, 100, Angle_Remote::now(), Angle_Remote::now() + iterations * period), expiry, 0, SmartData::SINGLE, 3);
        Angle_Local angl(3, expiry);
        angp.attach(&angl);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 4: {
        cout << "I'm an Acceleration Throttle (Actuator)";
        Acceleration_Remote_Proxy ap(Acceleration_Remote::Region(0, 0, 0, 100, Acceleration_Remote::now(), Acceleration_Remote::now() + iterations * period), expiry, 0, SmartData::SINGLE, 4);
        Acceleration_Local apl(4, expiry);
        ap.attach(&apl);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 5: {
        cout << "I'm a Break Intensity (Actuator)";
        Acceleration_Remote_Proxy abp(Acceleration_Remote::Region(0, 0, 0, 100, Acceleration_Remote::now(), Acceleration_Remote::now() + iterations * period), expiry, 0, SmartData::SINGLE, 5);
        Acceleration_Local abpl(5, expiry);
        abp.attach(&abpl);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 6:
        // cout << "I'm a HandBrake (Actuator)";
        break;
    case 7:
        // cout << "I'm a No Sky Image (Transformational)";
        break;
    case 8:
        // cout << "I'm a Resized Image (Transformational)";
        break;
    case 9: {
        cout << "I'm a Fake GPS (Sensor)";
        GPS3I gps(9, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 10: {
        cout << "I'm a Dynamics Array (Sensor)";
        Dynamics_Array dynarray(10, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 11: {
        cout << "I'm a Dynamics (Sensor)";
        Dynamics dyn(11, expiry, SmartData::ADVERTISED);
        cout << "Done!" << endl;
        while(true) {Delay(period);}
        break;
    }
    case 12: {
        cout << "I'm a Imitation_Learning_Model (Sensor)";
        Imitation_Learning_Model iml(12, expiry, SmartData::PRIVATE);
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

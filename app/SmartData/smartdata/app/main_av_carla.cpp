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
#include <execinfo.h>

#define Delay usleep

#ifdef IMU_HARDWARE
constexpr bool IMU_HARDWARE_ENABLED   = true;
using Longitudinal_Acceleration_Alias = LPMS_IMU_Longitudinal_Acceleration;
using Lateral_Acceleration_Alias      = LPMS_IMU_Lateral_Acceleration;
using Vertical_Acceleration_Alias     = LPMS_IMU_Vertical_Acceleration;
using Yaw_Rate_Alias                  = LPMS_IMU_Yaw_Rate;
using Pitch_Rate_Alias                = LPMS_IMU_Pitch_Rate;
using Roll_Rate_Alias                 = LPMS_IMU_Roll_Rate;
using Yaw_Alias                       = LPMS_IMU_Yaw;
using Dynamics_State_Alias            = Dynamics_State_KF_On_Hardware_SD;
#else
constexpr bool IMU_HARDWARE_ENABLED   = false;
using Longitudinal_Acceleration_Alias = Longitudinal_Acceleration;
using Lateral_Acceleration_Alias      = Lateral_Acceleration;
using Vertical_Acceleration_Alias     = Vertical_Acceleration;
using Yaw_Rate_Alias                  = Yaw_Rate;
using Pitch_Rate_Alias                = Pitch_Rate;
using Roll_Rate_Alias                 = Roll_Rate;
using Yaw_Alias                       = Yaw;
using Dynamics_State_Alias            = Dynamics_State;
#endif

#ifdef GNSS_HARDWARE
constexpr bool GNSS_HARDWARE_ENABLED = true;
using Longitude_Alias                = LPMS_GNSS_Longitude;
using Latitude_Alias                 = LPMS_GNSS_Latitude;
using Altitude_Alias                 = LPMS_GNSS_Altitude;
using Speed_Alias                    = LPMS_GNSS_Velocity;
#else
constexpr bool GNSS_HARDWARE_ENABLED = false;
using Longitude_Alias                = Longitude;
using Latitude_Alias                 = Latitude;
using Altitude_Alias                 = Altitude;
using Speed_Alias                    = Speed;
#endif

#ifdef CAMERA_HARDWARE
constexpr bool CAMERA_HARDWARE_ENABLED = true;
using Camera_Alias                     = OAK_D_Camera_Shared_Memory;
#else
constexpr bool CAMERA_HARDWARE_ENABLED = false;
using Camera_Alias                     = Camera_AV;
#endif

#ifdef LIDAR_HARDWARE
constexpr bool LIDAR_HARDWARE_ENABLED = true;
using LIDAR_Alias                     = VLP16_LIDAR;
#else
constexpr bool LIDAR_HARDWARE_ENABLED = false;
using LIDAR_Alias                     = LIDAR_AV;
#endif

static const int LAST_DEVICE     = 64;
static const UInt32 EXPIRY = 100000;

OStream cout;
List<pid_t>* daemons;
UDP_Socket* sdhandler_socket;
pid_t* my_pid;

SEU_SmartData* seu_sd;
Unit_Dev_Expiry::List* ud_list;
Monitoring* monitor;
MU_Arrival_Dep* dynamics;
MU_Arrival_Dep* object_rec_trac_cam;
MU_Arrival_Dep* object_rec_trac_lidar;
MU_Arrival_Dep* object_rec_trac_radar;
MU_Arrival_Dep* object_rec_trac_fuser;
MU_Arrival_Dep* visual_vibration_detector;
MU_Arrival_Dep* road_surface_condition_detector;
MU_Arrival_Dep* path_planning;
MU_Arrival_Dep* motion_planning;
MU_Arrival_Dep* throttle;
MU_Arrival_Dep* brake;
MU_Arrival_Dep* steering;
MU_Arrival_Dep* hand_brake;
MU_Arrival_Dep* reverse;
RSS_Safe_Distance* rss;
bool ended;
bool im_sink;

typedef IF<Traits<Project>::CAM_IS_TRANSFORMER, Object_Recognition_And_Tracking_Camera_Transformer, Object_Recognition_And_Tracking_Camera>::Result Camera_ORT;
typedef IF<Traits<Project>::LIDAR_IS_TRANSFORMER, Object_Recognition_And_Tracking_LIDAR_Transformer, Object_Recognition_And_Tracking_LIDAR>::Result LIDAR_ORT;
typedef IF<Traits<Project>::FUSER_IS_GROUND_TRUTH, Object_Recognition_And_Tracking_FUSER_Ground_Truth, Object_Recognition_And_Tracking_Fuser>::Result FUSER_ORT;

typedef IF<Traits<Project>::CAM_IS_TRANSFORMER, Visual_Vibration_Detection_Camera_File, Visual_Vibration_Detection_Camera>::Result Camera_VVD;

typedef IF<Traits<Project>::CAM_IS_TRANSFORMER, Road_Surface_Condition_Detection_Camera_File, Road_Surface_Condition_Detection_Camera>::Result Camera_RSCD;

// Declaratoin of all interests types that would be used explicitly in main
using RSCD_Proxy =      Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::ROAD_SURFACE_CONDITION)>>;
using Brake_Source_Proxy =      Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Acceleration | SmartData::Unit::F32)>>;
using Steer_Source_Proxy =      Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Angle | SmartData::Unit::F32)>>;
using Reverse_Source_Proxy =    Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Counter)>>;
using Hand_Brake_Source_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Counter)>>;
using Throttle_Source_Proxy   = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Acceleration | SmartData::Unit::F32)>>;

using Front_Stiffness_Proxy = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Stiffness | SmartData::Unit::F32)>>;
using Rear_Stiffness_Proxy  = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Stiffness | SmartData::Unit::F32)>>;
using Front_Damping_Proxy   = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Damping | SmartData::Unit::F32)>>;
using Rear_Damping_Proxy    = Interested_SmartData<SmartData::Unit::Wrap<(SmartData::Unit::Damping | SmartData::Unit::F32)>>;

void Usage();

void create_daemon(UInt32 dev, SmartData::Time period, SmartData::Time expiry, UInt32 iterations);

void clean_before_exit()
{
    pid_t* d;
    int status;
    while (!daemons->empty()) {
        d = daemons->remove_tail()->object();  // it is not necessary to send the kill signal, this will be done by sink...
        cout << "Cleaning daemon... pid=" << *d;
        if (im_sink) {
            cout << ", killing pid=" << *d;
            kill(*d, SIGKILL);
            waitpid(*d, &status, 0);
            cout << ", status=" << status;
        }
        delete d;
        cout << "...Daemon cleaned!" << endl;
    }
    if (im_sink)
        Delay(10000000); // wait for all daemons to be cleaned
    cout << "All daemons cleaned!" << endl;
    if (daemons)
        delete daemons;
    cout << "Daemons list deleted!" << endl;
    if (!Traits<Build>::no_connection_test) {
        if (sdhandler_socket)
            delete sdhandler_socket;
        cout << "SDHandler socket deleted!" << endl;
    }
    if (seu_sd != 0) {
        cout << "SEU SmartData deleting..." << endl;
        seu_sd->print_stats();
        delete seu_sd;
        cout << "SEU SmartData deleted!" << endl;
    }
    ended = true;
}

static const char* signal_name(int sig) {
    switch (sig) {
        case SIGINT:  return "SIGINT";
        case SIGTERM: return "SIGTERM";
        case SIGSEGV: return "SIGSEGV";
        case SIGABRT: return "SIGABRT";
        case SIGFPE:  return "SIGFPE";
        case SIGILL:  return "SIGILL";
        case SIGBUS:  return "SIGBUS";
        case SIGPIPE: return "SIGPIPE";
        case SIGHUP:  return "SIGHUP";
        case SIGQUIT: return "SIGQUIT";
        case SIGTRAP: return "SIGTRAP";
        case SIGKILL: return "SIGKILL";
        case SIGUSR1: return "SIGUSR1";
        case SIGUSR2: return "SIGUSR2";
        case SIGALRM: return "SIGALRM";
        default:      return "UNKNOWN";
    }
}

void sig_handler(int sig)
{
    switch (sig) {
        case SIGKILL:
            cout << "SINK called kill..." << endl;
        case SIGTERM:
        case SIGINT:
            cout << "Caught signal " << signal_name(sig) << "mypid=" << getpid() << ", cleaning before exit..." << endl;
            clean_before_exit();
            break;
            // exit(0);
        default:
            cout << "... wasn't expecting that! ERROR=" << signal_name(sig) << endl;
            void *array[10];
            size_t size;

            // get void*'s for all entries on the stack
            size = backtrace(array, 10);

            // print out all the frames to stderr
            fprintf(stderr, "Error: signal %d:\n", sig);
            backtrace_symbols_fd(array, size, STDERR_FILENO);

            clean_before_exit();
            // abort();
            break;
    }
}

void seu()
{
    im_sink = false;
    int fd;
    char log_file[24] = "logs/sniffer.log";
    fd                = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    dup2(fd, STDOUT_FILENO);

    cout << "TSTP::init()...";
    TSTP::init();
    cout << "done." << endl;
    
    seu_sd = new SEU_SmartData();
    cout << "SEU:" << seu_sd << endl;

    ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
    // IMU XYZ Acceleration
    ud_list->insert((new Unit_Dev_Expiry(Longitudinal_Acceleration_Alias::UNIT, 0, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Lateral_Acceleration_Alias::UNIT, 1, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Vertical_Acceleration_Alias::UNIT, 2, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Yaw_Rate_Alias::UNIT, 36, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Pitch_Rate_Alias::UNIT, 37, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Roll_Rate_Alias::UNIT, 38, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Drag::UNIT, 6, 100000))->link());

    #ifdef IMU_HARDWARE_ENABLED
        ud_list->insert((new Unit_Dev_Expiry(Speed_Alias::UNIT, 3, 1000000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Longitude_Alias::UNIT, 7, 1000000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Latitude_Alias::UNIT, 8, 1000000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Altitude_Alias::UNIT, 9, 1000000))->link());
    #else
        ud_list->insert((new Unit_Dev_Expiry(Speed_Alias::UNIT, 3, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Speed_Alias::UNIT, 4, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Speed_Alias::UNIT, 5, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Longitude_Alias::UNIT, 7, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Latitude_Alias::UNIT, 8, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Altitude_Alias::UNIT, 9, 100000))->link());
    #endif
    ud_list->insert((new Unit_Dev_Expiry(Gear::UNIT, 10, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Engine_RPM::UNIT, 11, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Mass::UNIT, 53, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Odometer::UNIT, 54, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Wheel_Telemetry::UNIT, 60, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Wheel_Telemetry::UNIT, 61, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Wheel_Telemetry::UNIT, 62, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Wheel_Telemetry::UNIT, 63, 100000))->link());
    // ud_list->insert((new Unit_Dev_Expiry(Battery_Charge::UNIT, 12, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Yaw_Alias::UNIT, 13, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Pitch::UNIT, 14, 100000))->link());
    // ud_list->insert((new Unit_Dev_Expiry(Roll::UNIT, 15, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Destination::UNIT, 41, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Dynamics_State_Alias::UNIT, 16, 100000))->link());

    // ud_list->insert((new Unit_Dev_Expiry(Camera_AV::UNIT, 17, 100000))->link());
    // ud_list->insert((new Unit_Dev_Expiry(LIDAR_AV::UNIT, 18, 100000))->link());
    // ud_list->insert((new Unit_Dev_Expiry(RADAR_AV_AMPERA::UNIT, 19, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Camera_ORT::UNIT, 20, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(LIDAR_ORT::UNIT, 21, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_RADAR::UNIT, 22, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(FUSER_ORT::UNIT, 23, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Map::UNIT, 24, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Path_Planning::UNIT, 29, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Steer_Actuator::UNIT, 31, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Throttle_Actuator::UNIT, 32, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Brake_Actuator::UNIT, 33, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Reverse_Actuator::UNIT, 34, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Hand_Brake_Actuator::UNIT, 35, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Front_Stiffness_Source::UNIT, 48, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Rear_Stiffness_Source::UNIT, 49, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Front_Damping_Source::UNIT, 50, 100000))->link());
    ud_list->insert((new Unit_Dev_Expiry(Rear_Damping_Source::UNIT, 51, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Camera_VVD::UNIT, 52, 100000))->link());

    ud_list->insert((new Unit_Dev_Expiry(Camera_RSCD::UNIT, 55, 100000))->link());


    cout << "SEU UD_List done!" << endl;

    monitor = new Monitoring(ud_list);
    cout << "Monitor created!" << endl;

    seu_sd->add_boolean_filter(monitor);

    if (Traits<Build>::verified) {
        // DYNAMICS
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....

        ud_list->insert((new Unit_Dev_Expiry(Longitudinal_Acceleration_Alias::UNIT, 0, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Lateral_Acceleration_Alias::UNIT, 1, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Vertical_Acceleration_Alias::UNIT, 2, 100000))->link());

        ud_list->insert((new Unit_Dev_Expiry(Speed_Alias::UNIT, 3, 100000))->link());

        ud_list->insert((new Unit_Dev_Expiry(Yaw_Rate_Alias::UNIT, 36, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Pitch_Rate_Alias::UNIT, 37, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Roll_Rate_Alias::UNIT, 38, 100000))->link());

        ud_list->insert((new Unit_Dev_Expiry(Longitude_Alias::UNIT, 7, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Latitude_Alias::UNIT, 8, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Altitude_Alias::UNIT, 9, 100000))->link());

        ud_list->insert((new Unit_Dev_Expiry(Yaw_Alias::UNIT, 13, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Pitch::UNIT, 14, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Roll::UNIT, 15, 100000))->link());

        dynamics = new MU_Arrival_Dep(ud_list, Dynamics_State_Alias::UNIT, 16, 100000, 100000);
        seu_sd->add_boolean_filter(dynamics);

        // CAMERA
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State_Alias::UNIT, 16, 100000))->link());
        if (!Traits<Project>::CAM_IS_TRANSFORMER) ud_list->insert((new Unit_Dev_Expiry(Camera_Alias::UNIT, 17, 100000))->link());

        object_rec_trac_cam = new MU_Arrival_Dep(ud_list, Object_Recognition_And_Tracking_Camera::UNIT, 20, 100000, 100000);
        seu_sd->add_boolean_filter(object_rec_trac_cam);

        // LIDAR
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....

        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State_Alias::UNIT, 16, 100000))->link());
        if (!Traits<Project>::LIDAR_IS_TRANSFORMER) ud_list->insert((new Unit_Dev_Expiry(LIDAR_Alias::UNIT, 18, 100000))->link());

        object_rec_trac_lidar = new MU_Arrival_Dep(ud_list, Object_Recognition_And_Tracking_LIDAR::UNIT, 21, 100000, 100000);
        seu_sd->add_boolean_filter(object_rec_trac_lidar);

        // FUSER
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....

        ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_Camera::UNIT, 20, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_LIDAR::UNIT, 21, 100000))->link());

        object_rec_trac_fuser = new MU_Arrival_Dep(ud_list, Object_Recognition_And_Tracking_Fuser::UNIT, 23, 100000, 100000);
        seu_sd->add_boolean_filter(object_rec_trac_fuser);

        // VIBRATION
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State::UNIT, 16, 100000))->link());
        if (!Traits<Project>::CAM_IS_TRANSFORMER) ud_list->insert((new Unit_Dev_Expiry(Camera_AV::UNIT, 17, 100000))->link());

        visual_vibration_detector = new MU_Arrival_Dep(ud_list, Camera_VVD::UNIT, 52, 100000, 100000);
        seu_sd->add_boolean_filter(visual_vibration_detector);

        // ROAD SURFACE CONDITION
        ud_list = new Unit_Dev_Expiry::List(); // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State::UNIT, 16, 100000))->link());
        if (!Traits<Project>::CAM_IS_TRANSFORMER)
            ud_list->insert((new Unit_Dev_Expiry(Camera_AV::UNIT, 17, 100000))->link());

        road_surface_condition_detector = new MU_Arrival_Dep(ud_list, Camera_RSCD::UNIT, 55, 150000, 150000);
        seu_sd->add_boolean_filter(road_surface_condition_detector);

        // PATH PLANNING
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....

        ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_Fuser::UNIT, 23, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Destination::UNIT, 41, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State_Alias::UNIT, 16, 100000))->link());

        path_planning = new MU_Arrival_Dep(ud_list, Path_Planning::UNIT, 29, 100000, 100000);
        seu_sd->add_boolean_filter(path_planning);

        // MOTION PLANNING
        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....

        ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_Fuser::UNIT, 23, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Path_Planning::UNIT, 29, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State_Alias::UNIT, 16, 100000))->link());

        motion_planning = new MU_Arrival_Dep(ud_list, Motion_Planning::UNIT, 30, 100000, 100000);
        seu_sd->add_boolean_filter(motion_planning);

        // // Actuators

        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());
        steering = new MU_Arrival_Dep(ud_list, Steer_Actuator::UNIT, 31, 100000, 100000);
        seu_sd->add_boolean_filter(steering);

        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());
        throttle = new MU_Arrival_Dep(ud_list, Throttle_Actuator::UNIT, 32, 100000, 100000);
        seu_sd->add_boolean_filter(throttle);

        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());
        brake = new MU_Arrival_Dep(ud_list, Brake_Actuator::UNIT, 33, 100000, 100000);
        seu_sd->add_boolean_filter(brake);

        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());
        reverse = new MU_Arrival_Dep(ud_list, Reverse_Actuator::UNIT, 34, 100000, 100000);
        seu_sd->add_boolean_filter(reverse);

        ud_list = new Unit_Dev_Expiry::List();  // List do not have desctructors, we need to delete here or to implement it on lists....
        ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());
        hand_brake = new MU_Arrival_Dep(ud_list, Hand_Brake_Actuator::UNIT, 35, 100000, 100000);
        seu_sd->add_boolean_filter(hand_brake);

        // // RSS
        Road_Parameters rp = Road_Parameters(0,0,0,0,0);
        rp.set_default();
        ud_list = new Unit_Dev_Expiry::List(); // deleted at bf destructor
        ud_list->insert((new Unit_Dev_Expiry(Dynamics_State::UNIT, 16, 100000))->link());
        ud_list->insert((new Unit_Dev_Expiry(Object_Recognition_And_Tracking_Fuser::UNIT, 23, 100000))->link());
        //ud_list->insert((new Unit_Dev_Expiry(Motion_Planning::UNIT, 30, 100000))->link());
        rss = new RSS_Safe_Distance(ud_list, &rp, &rp, 100000);
        seu_sd->add_boolean_filter(rss);
    }

    cout << "Waiting for data!\nLog Start:" << endl;
    while (!ended) {
        Delay(15 * 1000000);
    }
}

int create_forked(UInt32 dev, UInt32 period, UInt32 iterations, UInt32 expiry) {
    cout << "Received: dev="<< dev << ",p=" << period << ",e=" << expiry << ",i=" << iterations << endl;
    if (dev > LAST_DEVICE) {
        cout << dev << " unsupported_device!" << endl;
        return -1;
    }
    my_pid = new pid_t(fork()); // maybe change this to thread later
    if (*my_pid == 0) {
        im_sink = false;
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
            dup2(fd, STDERR_FILENO);
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
    }
    return 1;
}


int main(int argc, char* argv[])
{
    cout << "Starting system abstractions..." << endl;
    cout << "Thread::init()...";
    Thread::init();
    cout << "done." << endl;
    daemons = new List<pid_t>();
    im_sink = false;
    ended = false;
    
    if (argc == 2 && argv[1][1] == 'e') {  // "seu"
        signal(SIGKILL, sig_handler);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        signal(SIGABRT, sig_handler);
        signal(SIGSEGV, sig_handler);
        signal(SIGFPE, sig_handler);
        signal(SIGILL, sig_handler);
        cout << "Creating SEU: ";
        my_pid = new pid_t(fork());
        if (*my_pid == 0) {
            seu();
            TSTP::finish();
            cout << "TSTP finished!" << endl;
            Thread::finish();
            cout << "Threads finished!" << endl;
            cout << "Goodbye..." << endl;
            return 0;
        } else {
            cout << "SEU PID=" << *my_pid << endl;
            daemons->insert(new List<pid_t>::Element(my_pid));
        }
    } else if (argc > 2) {
        cout << "Wrong usage..." << endl;
        Usage();
        return -1;
    }

    if (Traits<Build>::no_connection_test) {
        cout << "SmartDataHandler is up and will create children processes for test..." << endl;
        int ret = 0;
        UInt32 period = 100000;
        UInt32 iterations = 100000;
        UInt32 expiry = 100000;
        UInt32 dev;
        for (dev = 0; dev <= LAST_DEVICE; dev++)
        {
            ret = create_forked(dev, period, iterations, expiry);
            if ( ret != 1) {
                break;
            }
        }
        while(!ended) { Delay(period*10); }
    } else {
        sdhandler_socket = new UDP_Socket(-1, false);
        UInt32 dev;
        UInt32 period;
        UInt32 iterations;
        UInt32 expiry;
        const unsigned char* data;
        cout << "SmartDataHandler is up and waiting for commands..." << endl;

        while (true) {
            data = sdhandler_socket->receive();  // allways a 1B+4*4B --> 'q' or 'c' followed by 4 uint32
            cout << "I've read changes=" << data << endl;

            if (data[0] == 'q') {
                break;
            } else {
                dev        = *reinterpret_cast<UInt32*>(const_cast<unsigned char*>(&data[1]));
                period     = *reinterpret_cast<UInt32*>(const_cast<unsigned char*>(&data[5]));
                expiry     = *reinterpret_cast<UInt32*>(const_cast<unsigned char*>(&data[9]));
                iterations = *reinterpret_cast<UInt32*>(const_cast<unsigned char*>(&data[13]));
                cout << "Received: dev=" << dev << ",p=" << period << ",e=" << expiry << ",i=" << iterations << endl;
                if (dev > LAST_DEVICE) {
                    cout << dev << " unsupported_device!" << endl;
                    continue;
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
            }
        }
    }
    TSTP::finish();
    cout << "TSTP finished!" << endl;
    Thread::finish();
    cout << "Threads finished!" << endl;
    cout << "Goodbye..." << endl;
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
    switch (dev) {
        case 0: {
            if (IMU_HARDWARE_ENABLED && GNSS_HARDWARE_ENABLED) {
                cout << "I'm a longitude (Sensor w/dev=7)\n";
                Longitude_Alias longitude(7, 1000000, SmartData::ADVERTISED);

                cout << "I'm a latitude (Sensor w/dev=8)\n";
                Latitude_Alias latitude(8, 1000000, SmartData::ADVERTISED);

                cout << "I'm an altitude (Sensor w/dev=9)\n";
                Altitude_Alias altitude(9, 1000000, SmartData::ADVERTISED);

                cout << "I'm a Speed X (Sensor w/dev=3)\n";
                Speed_Alias s(3, 1000000, SmartData::ADVERTISED);

                cout << "I'm a longitudinal acceleration (Sensor w/dev=0)\n";
                Longitudinal_Acceleration_Alias longitudinal_acceleration(0, expiry, SmartData::ADVERTISED);

                cout << "I'm a lateral acceleration (Sensor w/dev=1)\n";
                Lateral_Acceleration_Alias lateral_acceleration(1, expiry, SmartData::ADVERTISED);

                cout << "I'm a vertical acceleration (Sensor w/dev=2)\n";
                Vertical_Acceleration_Alias vertical_acceleration(2, expiry, SmartData::ADVERTISED);

                cout << "I'm a yaw rate (Sensor w/dev=36)\n";
                Yaw_Rate_Alias yaw_rate(36, expiry, SmartData::ADVERTISED);

                cout << "I'm a pitch rate (Sensor w/dev=37)\n";
                Pitch_Rate_Alias pitch_rate(37, expiry, SmartData::ADVERTISED);

                cout << "I'm a roll rate (Sensor w/dev=38)\n";
                Roll_Rate_Alias roll_rate(38, expiry, SmartData::ADVERTISED);

                cout << "I'm a yaw (Sensor w/dev=13)\n";
                Yaw_Alias yaw(13, expiry, SmartData::ADVERTISED);
                while (!ended) Delay(period);
            } else {
                cout << "I'm a longitudinal acceleration (Sensor w/dev=0)\n";
                Longitudinal_Acceleration_Alias longitudinal_acceleration(0, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 1: {
            if (!IMU_HARDWARE_ENABLED) {
                cout << "I'm a lateral acceleration (Sensor w/dev=1)\n";
                Lateral_Acceleration_Alias lateral_acceleration(1, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 2: {
            if (!IMU_HARDWARE_ENABLED) {
                cout << "I'm a vertical acceleration (Sensor w/dev=2)";
                Vertical_Acceleration_Alias vertical_acceleration(2, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while (!ended) Delay(period);
            }
            break;
        }
        case 36: {
            if (!IMU_HARDWARE_ENABLED) {
                cout << "I'm a yaw rate (Sensor w/dev=36)";
                Yaw_Rate_Alias yaw_rate(36, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 37: {
            if (!IMU_HARDWARE_ENABLED) {
                cout << "I'm a pitch rate (Sensor w/dev=37)";
                Pitch_Rate_Alias pitch_rate(37, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 38: {
            if (!IMU_HARDWARE_ENABLED) {
                cout << "I'm a roll rate (Sensor w/dev=38) -- not supported";
                Roll_Rate_Alias roll_rate(38, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 3: {
            if (!GNSS_HARDWARE_ENABLED) {
                cout << "I'm a Speed X (Sensor w/dev=3)";
                Speed s(3, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 4: {
            cout << "I'm a Speed Y (Sensor w/dev=4)";
            Speed s(4, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 5: {
            cout << "I'm a Speed Z (Sensor w/dev=5)";
            Speed s(5, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 6: {
            cout << "I'm an Drag (Sensor w/dev=6)";
            Drag drag(6, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 7: {
            if (!GNSS_HARDWARE_ENABLED) {
                cout << "I'm a longitude (Sensor w/dev=7)";
                Longitude_Alias longitude(7, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 8: {
            if (!GNSS_HARDWARE_ENABLED) {
                cout << "I'm a latitude (Sensor w/dev=8)";
                Latitude_Alias latitude(8, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 9: {
            if (!GNSS_HARDWARE_ENABLED) {
                cout << "I'm an altitude (Sensor w/dev=9)";
                Altitude_Alias altitude(9, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 10: {
            cout << "I'm a Gear (Sensor w/dev=10)";
            Gear gear(10, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 11: {
            cout << "I'm an Engine RPM (Sensor w/dev=11)";
            Engine_RPM engine_rpm(11, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 12: {
            cout << "Battery State of Charge (Sensor w/dev=12)... -- not supported";
            break;
        }
        case 13: {
            if (!IMU_HARDWARE_ENABLED) {
                cout << "I'm a yaw (Sensor w/dev=13)";
                Yaw_Alias yaw(13, expiry, SmartData::ADVERTISED);
                cout << "Done!" << endl;
                while(!ended) Delay(period);
            }
            break;
        }
        case 14: {
            cout << "I'm a pitch (Sensor w/dev=14)";
            Pitch pitch(14, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 15: {
            cout << "I'm a roll (Sensor w/dev=15)";
            Roll roll(15, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 41: {
            cout << "Destination (Transformer w/dev=41)";
            Destination dyn(41, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 16: {
            cout << "Dynamics_State (Transformer w/dev=16) -- Kalman Filters...";
            Dynamics_State_Alias dyn(16, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 17: {
            cout << "Camera Image (Sensor w/dev=17)...";
            Camera_Alias cam(17, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 18: {
            cout << "LIDAR Cloud Point (Sensor w/dev=18)...";
            LIDAR_Alias lid(18, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 19: {
            cout << "RADAR Cloud Point (Sensor w/dev=19)... -- not supported -- expecting a fake implementation";
            RADAR_AV rad(19, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 20: {
            cout << "CAMERA Object Recognition and Tracking (dev=20)...";
            Camera_ORT c(20, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 21: {
            cout << "LIDAR Object Recognition and Tracking (dev=21)...";
            LIDAR_ORT l(21, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 22: {
            cout << "RADAR Object Recognition and Tracking (Transformer w/dev=22)...";
            Object_Recognition_And_Tracking_RADAR r(22, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 23: {
            cout << "Fuser Object Recognition and Tracking (Transformer w/dev=23)...";
            FUSER_ORT f(23, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 24: {
            cout << "Map (Sensor w/dev=24)... not implemented yet";
            break;
            // Map m(24, expiry, SmartData::ADVERTISED);
            // cout << "Done!" << endl;
            // while(true) {Delay(period);}
            break;
        }
        case 25: {
            cout << "Map Orientation (Sensor w/dev=25)... -- not supported";
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
            while(!ended) Delay(period);
            break;
        }
        case 30: {
            cout << "Motion Planning (Transformerw/dev=30)...";
            Motion_Planning c(30, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 31: {
            cout << "Steering (Actuator)";
            Steer_Source_Proxy sp(Steer_Source_Proxy::Region(0, 0, 0, 100, Steer_Source_Proxy::now(), Steer_Source_Proxy::now() + iterations * period), expiry, 0,
                                  SmartData::SINGLE, SmartData::ANY, 31);
            Steer_Actuator sa(31, expiry);
            sp.attach(&sa);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 32: {
            cout << "Throttle (Actuator)";
            Throttle_Source_Proxy tp(Throttle_Source_Proxy::Region(0, 0, 0, 100, Throttle_Source_Proxy::now(), Throttle_Source_Proxy::now() + iterations * period), expiry, 0,
                                     SmartData::SINGLE, SmartData::ANY, 32);
            Throttle_Actuator ta(32, expiry);
            tp.attach(&ta);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 33: {
            cout << "Brake (Actuator)";
            Brake_Source_Proxy bp(Brake_Source_Proxy::Region(0, 0, 0, 100, Brake_Source_Proxy::now(), Brake_Source_Proxy::now() + iterations * period), expiry, 0,
                                  SmartData::SINGLE, SmartData::ANY, 33);
            Brake_Actuator ba(33, expiry);
            bp.attach(&ba);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 34: {
            cout << "Reverse (Actuator)";
            Reverse_Source_Proxy rp(Reverse_Source_Proxy::Region(0, 0, 0, 100, Reverse_Source_Proxy::now(), Reverse_Source_Proxy::now() + iterations * period), expiry, 0,
                                    SmartData::SINGLE, SmartData::ANY, 34);
            Reverse_Actuator ra(34, expiry);
            rp.attach(&ra);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 35: {
            cout << "Hand Brake (Actuator)";
            Hand_Brake_Source_Proxy hp(Hand_Brake_Source_Proxy::Region(0, 0, 0, 100, Hand_Brake_Source_Proxy::now(), Hand_Brake_Source_Proxy::now() + iterations * period), expiry,
                                       0, SmartData::SINGLE, SmartData::ANY, 35);
            Hand_Brake_Actuator ha(35, expiry);
            hp.attach(&ha);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 39: {
            cout << "High-Level Control (Transformer)";
            High_Level_Control c(39, expiry);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 47: {
            cout << "Suspension Parameters Predictor (Transformer)...";
            Suspension_Predictor s(47, expiry);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 48: {
            cout << "Suspension Parameters Predictor (Proxy)...";
            Front_Stiffness_Proxy fs(Front_Stiffness_Proxy::Region(0, 0, 0, 100, Front_Stiffness_Proxy::now(), INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 48);
            Rear_Stiffness_Proxy rs(Rear_Stiffness_Proxy::Region(0, 0, 0, 100, Rear_Stiffness_Proxy::now(), INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 49);
            Front_Damping_Proxy fd(Front_Damping_Proxy::Region(0, 0, 0, 100, Front_Damping_Proxy::now(), INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 50);
            Rear_Damping_Proxy rd(Rear_Damping_Proxy::Region(0, 0, 0, 100, Rear_Damping_Proxy::now(), INFINITE), EXPIRY, 0, SmartData::SINGLE, SmartData::ANY, 51);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }

        case 52: {
            cout << "Visual Vibration Detection...";
            Camera_VVD s(52, expiry);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }

        case 53: {
            cout << "Mass (Sensor w/dev=49)";
            Mass mass(53, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 54: {
            cout << "Odometer (Sensor w/dev=50)";
            Odometer odometer(54, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 55: {
            cout << "Road Surface Condition Detection...";
            Camera_RSCD crscd(55, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            RSCD_Proxy rscdp(RSCD_Proxy::Region(0, 0, 0, 100, RSCD_Proxy::now(), RSCD_Proxy::now() + iterations * period), expiry, 0, SmartData::SINGLE, SmartData::ANY, 55);
            while(!ended) Delay(period); 
            break;
        }
        case 60: {
            cout << "Wheel Speeds (Sensor w/dev=60/61/62/63)";
            Wheel_Telemetry wt_fl(60, expiry, SmartData::ADVERTISED);
            Wheel_Telemetry wt_fr(61, expiry, SmartData::ADVERTISED);
            Wheel_Telemetry wt_rl(62, expiry, SmartData::ADVERTISED);
            Wheel_Telemetry wt_rr(63, expiry, SmartData::ADVERTISED);
            cout << "Done!" << endl;
            while(!ended) Delay(period);
            break;
        }
        case 61:
        case 62:
        case 63:
            cout << "Device already start with dev=60!" << endl;
            break;
        default:
            cout << "Invalid Device, waiting for kill signal!" << endl;
            break;
        }
    if (!ended)
        clean_before_exit();
    TSTP::finish();
    cout << "TSTP finished!" << endl;
    Thread::finish();
    cout << "Threads finished!" << endl;
    cout << "Good bye!" << endl;
    exit(0);
}

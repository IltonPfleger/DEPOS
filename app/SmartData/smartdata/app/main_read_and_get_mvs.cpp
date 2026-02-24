#define AV_PROJECT
#define CARLA_V2_PROJECT
#define AV_MODULES_PYTHON // for dynamics 

#include <main_traits.h>
#include <network/tstp/tstp.h>
#include <architecture/cpu.h>
#include <transducer.h>
#include <transformer.h>
#include <smartdata.h>
#include <boolean_filters.h>
#include <utility/geometry.h>

#define Delay usleep

void clean_before_exit() {
    TSTP::finish();
    Thread::finish();
}

OStream cout;
void sig_handler(int sig) {
    switch (sig) {
    case SIGKILL:
    case SIGTERM:
        cout << "Called kill... --> my_pid=" << getpid() << endl;
        clean_before_exit();
        exit(0);
    default:
        cout << stderr << "... wasn't expecting that!" << endl;
        abort();
    }
}

class Actuate : public Observer
{
public:
    Actuate(SmartData::Time expiry) {
        // IMU
        _longitudinal_acceleration = new Longitudinal_Acceleration(0, expiry, SmartData::ADVERTISED);
        _longitudinal_acceleration->attach(this);
        _lateral_acceleration = new Lateral_Acceleration(1, expiry, SmartData::ADVERTISED);
        _lateral_acceleration->attach(this);
        _vertical_acceleration = new Vertical_Acceleration(2, expiry, SmartData::ADVERTISED);
        _vertical_acceleration->attach(this);
        _yaw_rate = new Yaw_Rate(36, expiry, SmartData::ADVERTISED);
        _yaw_rate->attach(this);
        _pitch_rate = new Pitch_Rate(37, expiry, SmartData::ADVERTISED);
        _pitch_rate->attach(this);
        _roll_rate = new Roll_Rate(38, expiry, SmartData::ADVERTISED);
        _roll_rate->attach(this);
        _heading = new Yaw(13, expiry, SmartData::ADVERTISED);
        _heading->attach(this);

        // GPS
        _longitude = new Longitude(7, expiry, SmartData::ADVERTISED);
        _longitude->attach(this);
        _latitude = new Latitude(8, expiry, SmartData::ADVERTISED);
        _latitude->attach(this);
        _elevation = new Altitude(9, expiry, SmartData::ADVERTISED);
        _elevation->attach(this);

        // Destination
        _destination = new Destination(41, expiry, SmartData::ADVERTISED); // MV of destination
        _destination->attach(this);

        // Camera
        _camera = new Camera_AV(17, expiry, SmartData::ADVERTISED);
        _camera->attach(this);

        // LIDAR
        _lidar = new LIDAR_AV(18, expiry, SmartData::ADVERTISED);
        _lidar->attach(this);

        // MV
        // Declars interest into all dynamics above except Camera and Lidar
        // requires starting external/AV_Modules_SD/dynamics_state.py
        _ego_mv = new Dynamics_State(16, expiry, SmartData::ADVERTISED);
        _ego_mv->attach(this);

        // Actuators
        _steer_actuator = new Steer_Actuator(31, expiry);
        _throttle_actuator = new Throttle_Actuator(32, expiry);
        _brake_actuator = new Brake_Actuator(33, expiry);
        _reverse_actuator = new Reverse_Actuator(34, expiry);
        _hand_brake_actuator = new Hand_Brake_Actuator(35, expiry);
        _last_actuation = _longitudinal_acceleration->now();
    }

    ~Actuate() {}


    void update(Observed *obs) {
        cout << "New data arrived...";
        if (_longitudinal_acceleration->when() > _last_actuation &&
            _lateral_acceleration->when() > _last_actuation &&
            _vertical_acceleration->when() > _last_actuation &&
            _yaw_rate->when() > _last_actuation &&
            _pitch_rate->when() > _last_actuation &&
            _roll_rate->when() > _last_actuation &&
            _heading->when() > _last_actuation &&
            _longitude->when() > _last_actuation &&
            _latitude->when() > _last_actuation &&
            _elevation->when() > _last_actuation &&
            _destination->when() > _last_actuation &&
            _camera->when() > _last_actuation &&
            _lidar->when() > _last_actuation && 
            _ego_mv->when() > _last_actuation) // When everyone is updated 
        {
            _last_actuation = _longitudinal_acceleration->now();
            print_sensors();
            // Do something with data here to produce actuation
            *_steer_actuator = 0;
            *_throttle_actuator = 0.5;
            *_brake_actuator = 0;
            *_reverse_actuator = 0;
            *_hand_brake_actuator = 0;
        } else {
            cout << "waiting for all data to be updated" << endl;
        }   
    }

    void print_sensors() {
        cout << "Longitudinal Acceleration="    << *_longitudinal_acceleration << endl;
        cout << "Lateral Acceleration="         << *_lateral_acceleration << endl;
        cout << "Vertical Acceleration="        << *_vertical_acceleration << endl;
        cout << "Yaw Rate="                     << *_yaw_rate << endl;
        cout << "Pitch Rate="                   << *_pitch_rate << endl;
        cout << "Roll Rate="                    << *_roll_rate << endl;
        cout << "Heading="                      << *_heading << endl;
        cout << "Longitude="                    << *_longitude << endl;
        cout << "Latitude="                     << *_latitude << endl;
        cout << "Altitude="                     << *_elevation << endl;

        cout << "Destination={\n\t";
        print_destination();
        cout << "}" << endl;
        cout << "EGO MV={\n\t";
        print_ego_mv();
        cout << "}" << endl;
    }

    void print_destination() {
        UInt32 size = 4;
        Destination::Value _value = *_destination;
        UInt32 index = 1; // skip first byte as it is used to tell if it is valid or not
        // Speed XYZ (12B), Acceleration XYZ (12B), YPR Rate (12B), Location XYZ (12B), YPR (12B), Steer(4B), Battery State of Charge (4B), Engine RPM (4B), Gear (4B)        
        cout << "lon="     << *reinterpret_cast<int *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "lat="     << *reinterpret_cast<int *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "alt="     << *reinterpret_cast<int *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "speed="     << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "heading="     << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "yawr="   << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "accel=" << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "id="  << *reinterpret_cast<UInt64 *>(&_value[index]) << ",\n\t";
        index += size*2;
        cout << "uncertainty="    << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "class="    << *reinterpret_cast<int *>(&_value[index]) << "\n\t";
    }

    void print_ego_mv() {
        UInt32 size = 4;
        Dynamics_State::Value _value = *_ego_mv;
        UInt32 index = 1; // skip first byte as it is used to tell if it is valid or not
        // Speed XYZ (12B), Acceleration XYZ (12B), YPR Rate (12B), Location XYZ (12B), YPR (12B), Steer(4B), Battery State of Charge (4B), Engine RPM (4B), Gear (4B)        
        cout << "lon="     << *reinterpret_cast<int *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "lat="     << *reinterpret_cast<int *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "alt="     << *reinterpret_cast<int *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "speed="     << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "heading="     << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "yawr="   << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "accel=" << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "id="  << *reinterpret_cast<UInt64 *>(&_value[index]) << ",\n\t";
        index += size*2;
        cout << "uncertainty="    << *reinterpret_cast<float *>(&_value[index]) << ",\n\t";
        index += size;
        cout << "class="    << *reinterpret_cast<int *>(&_value[index]) << "\n\t";
    }

    // IMU
    Longitudinal_Acceleration *_longitudinal_acceleration;
    Lateral_Acceleration *_lateral_acceleration;
    Vertical_Acceleration *_vertical_acceleration;
    Yaw_Rate *_yaw_rate;
    Pitch_Rate *_pitch_rate;
    Roll_Rate *_roll_rate;
    Yaw *_heading;

    // GPS
    Longitude *_longitude;
    Latitude *_latitude;
    Altitude *_elevation;

    // Destination
    Destination *_destination; // MV of destination

    // Camera
    Camera_AV *_camera;

    // LIDAR
    LIDAR_AV *_lidar;

    // Transformers
    Dynamics_State *_ego_mv;

    // Actuators
    Steer_Actuator *_steer_actuator;
    Throttle_Actuator *_throttle_actuator;
    Brake_Actuator *_brake_actuator;
    Reverse_Actuator *_reverse_actuator;
    Hand_Brake_Actuator *_hand_brake_actuator;

    SmartData::Time _last_actuation;

};

int main(int argc, char* argv[])
{
    cout << "Starting system abstractions..." << endl;
    cout << "Thread::init()...";
	Thread::init();
    cout << "done." << endl;

    cout << "TSTP::init()...";
	TSTP::init();
    cout << "done." << endl;

    Actuate a(100000);
    while(true) { Delay(100*1000000); }; // sleep for a long time 
    Thread::finish();

    return 0;
}
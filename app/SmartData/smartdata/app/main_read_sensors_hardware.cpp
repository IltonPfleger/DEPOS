#define AV_PROJECT  // hardware defines are made when compile flag "hardware" is used
#define ABS(x) ((x) > 0 ? (x) : (-x))
#include <architecture/cpu.h>
#include <boolean_filters.h>
#include <fcntl.h>
#include <main_traits.h>
#include <math.h>
#include <network/tstp/tstp.h>
#include <smartdata.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <transducer.h>
#include <transformer.h>
#include <utility/geometry.h>

#ifdef LIDAR_VIEWER
    #include <utility/lidar_viewer.h>
    #define IF_LIDAR_VIEWER(x) x
#else
    #define IF_LIDAR_VIEWER(x)
#endif

#ifdef CAMERA_VIEWER
    #include <opencv2/opencv.hpp>
    #define IF_CAMERA_VIEWER(x) x
#else
    #define IF_CAMERA_VIEWER(x)
#endif

class IMU_Actuate : public Observer
{
    static const UInt64 SIGNATURE = Traits<Project>::EGO_ID;

   public:
    IMU_Actuate(SmartData::Time expiry)
    {
        _FILE = open("bin/IMU-GNSS/imu-gnss.log", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        dup2(_FILE, STDOUT_FILENO);
        std::cout << "Log Start:\n" << std::endl;
        pthread_mutex_init(&_mutex, NULL);
        _imu_a_x        = new LPMS_IMU_Longitudinal_Acceleration(0, expiry, SmartData::PRIVATE);
        _imu_a_y        = new LPMS_IMU_Lateral_Acceleration(1, expiry, SmartData::PRIVATE);
        _imu_a_z        = new LPMS_IMU_Vertical_Acceleration(2, expiry, SmartData::PRIVATE);
        _velocity       = new LPMS_GNSS_Velocity(3, expiry, SmartData::PRIVATE);
        _imu_g_y        = new LPMS_IMU_Yaw_Rate(36, expiry, SmartData::PRIVATE);
        _imu_g_p        = new LPMS_IMU_Pitch_Rate(37, expiry, SmartData::PRIVATE);
        _imu_g_r        = new LPMS_IMU_Roll_Rate(38, expiry, SmartData::PRIVATE);
        _imu_heading    = new LPMS_IMU_Yaw(13, expiry, SmartData::PRIVATE);
        _gnss_longitude = new LPMS_GNSS_Longitude(7, expiry, SmartData::PRIVATE);
        _gnss_latitude  = new LPMS_GNSS_Latitude(8, expiry, SmartData::PRIVATE);
        _gnss_altitude  = new LPMS_GNSS_Altitude(9, expiry, SmartData::PRIVATE);
        _imu_a_x->attach(this);
        _imu_a_y->attach(this);
        _imu_a_z->attach(this);
        _imu_g_y->attach(this);
        _imu_g_p->attach(this);
        _imu_g_r->attach(this);
        _imu_heading->attach(this);
        _gnss_longitude->attach(this);
        _gnss_latitude->attach(this);
        _gnss_altitude->attach(this);
        _velocity->attach(this);
        _imu_last_actuation  = _imu_a_x->now();
        _gnss_last_actuation = _imu_a_x->now();
    }

    ~IMU_Actuate() { close(_FILE); }

    void update(Observed *obs)
    {
        fprintf(stderr, "New imu/gnss data arrived\n");

        if (_gnss_longitude->when() > _gnss_last_actuation && _gnss_latitude->when() > _gnss_last_actuation && _gnss_altitude->when() > _gnss_last_actuation) {
            _gnss_last_actuation = _gnss_longitude->now();
            pthread_mutex_lock(&_mutex);
            db<SmartData>(LOGGER) << "(u=" << _gnss_longitude->unit();
            std::cout << "=>" << (UInt32)_gnss_longitude->unit() << ",d=" << 7 << ",t=" << _gnss_longitude->when() << ",sig=" << IMU_Actuate::SIGNATURE
                      << ")={" << *_gnss_longitude << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _gnss_latitude->unit();
            std::cout << "=>" << (UInt32)_gnss_latitude->unit() << ",d=" << 8 << ",t=" << _gnss_latitude->when() << ",sig=" << IMU_Actuate::SIGNATURE
                      << ")={" << *_gnss_latitude << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _gnss_altitude->unit();
            std::cout << "=>" << (UInt32)_gnss_altitude->unit() << ",d=" << 9 << ",t=" << _gnss_altitude->when() << ",sig=" << IMU_Actuate::SIGNATURE
                      << ")={" << *_gnss_altitude << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _velocity->unit();
            std::cout << "=>" << (UInt32)_velocity->unit() << ",d=" << 3 << ",t=" << _velocity->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_velocity << "}";
            std::cout << "\n";

            std::cout << "\n" << std::endl;
            pthread_mutex_unlock(&_mutex);
        }

        if (_imu_a_x->when() > _imu_last_actuation && _imu_a_y->when() > _imu_last_actuation && _imu_a_z->when() > _imu_last_actuation &&
            _imu_g_y->when() > _imu_last_actuation && _imu_g_p->when() > _imu_last_actuation && _imu_g_r->when() > _imu_last_actuation &&
            _imu_heading->when() > _imu_last_actuation) {
            _imu_last_actuation = _imu_a_x->now();
            pthread_mutex_lock(&_mutex);

            db<SmartData>(LOGGER) << "(u=" << _imu_a_x->unit();
            std::cout << "=>" << (UInt32)_imu_a_x->unit() << ",d=" << 0 << ",t=" << _imu_a_x->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_imu_a_x << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _imu_a_y->unit();
            std::cout << "=>" << (UInt32)_imu_a_y->unit() << ",d=" << 1 << ",t=" << _imu_a_y->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_imu_a_y << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _imu_a_z->unit();
            std::cout << "=>" << (UInt32)_imu_a_z->unit() << ",d=" << 2 << ",t=" << _imu_a_z->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_imu_a_z << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _imu_g_y->unit();
            std::cout << "=>" << (UInt32)_imu_g_y->unit() << ",d=" << 36 << ",t=" << _imu_g_y->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_imu_g_y << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _imu_g_p->unit();
            std::cout << "=>" << (UInt32)_imu_g_p->unit() << ",d=" << 37 << ",t=" << _imu_g_p->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_imu_g_p << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _imu_g_r->unit();
            std::cout << "=>" << (UInt32)_imu_g_r->unit() << ",d=" << 38 << ",t=" << _imu_g_r->when() << ",sig=" << IMU_Actuate::SIGNATURE << ")={"
                      << *_imu_g_r << "}";
            std::cout << "\n";

            db<SmartData>(LOGGER) << "(u=" << _imu_heading->unit();
            std::cout << "=>" << (UInt32)_imu_heading->unit() << ",d=" << 13 << ",t=" << _imu_heading->when() << ",sig=" << IMU_Actuate::SIGNATURE
                      << ")={" << *_imu_heading << "}";
            std::cout << "\n";

            std::cout << "\n" << std::endl;
            pthread_mutex_unlock(&_mutex);
        };
    }

   private:
    // LPMS_IG1P_IMU
    LPMS_IMU_Longitudinal_Acceleration *_imu_a_x;
    LPMS_IMU_Lateral_Acceleration *_imu_a_y;
    LPMS_IMU_Vertical_Acceleration *_imu_a_z;
    LPMS_IMU_Yaw_Rate *_imu_g_y;
    LPMS_IMU_Pitch_Rate *_imu_g_p;
    LPMS_IMU_Roll_Rate *_imu_g_r;
    LPMS_IMU_Yaw *_imu_heading;
    // GNSS
    LPMS_GNSS_Longitude *_gnss_longitude;
    LPMS_GNSS_Latitude *_gnss_latitude;
    LPMS_GNSS_Altitude *_gnss_altitude;
    // Velocity
    LPMS_GNSS_Velocity *_velocity;
    //__
    SmartData::Time _imu_last_actuation;
    SmartData::Time _gnss_last_actuation;
    int _FILE;
    pthread_mutex_t _mutex;
};

class Camera_Actuate : public Observer
{
   public:
    Camera_Actuate(SmartData::Time expiry)
    {
        _camera = new OAK_D_Camera(17, expiry, SmartData::PRIVATE);
        _camera->attach(this);
        _last_actuation = _camera->now();
    }

    ~Camera_Actuate() {}

    void render_image(unsigned char *image)
    {
        IF_CAMERA_VIEWER(cv::Mat mat = cv::Mat(1200, 1920, CV_8UC3, image));
        IF_CAMERA_VIEWER(cv::namedWindow("C", cv::WINDOW_NORMAL));
        IF_CAMERA_VIEWER(cv::resizeWindow("C", 500, 500));
        IF_CAMERA_VIEWER(cv::imshow("C", mat));
        IF_CAMERA_VIEWER(cv::waitKey(1));
    }

    void update(Observed *obs)
    {
        std::cout << "New camera data arrived..." << std::endl;
        if (_camera->when() > _last_actuation) render_image(&((unsigned char *)(&(*_camera)))[101]);
    }

   private:
    OAK_D_Camera *_camera;
    SmartData::Time _last_actuation;
};

class LIDAR_Actuate : public Observer
{
   public:
    LIDAR_Actuate(SmartData::Time expiry)
    {
        _lidar = new VLP16_LIDAR(18, expiry, SmartData::PRIVATE);
        _lidar->attach(this);
        _last_actuation = _lidar->now();
    }

    ~LIDAR_Actuate() {}

    void update(Observed *obs)
    {
        std::cout << "New lidar data arrived..." << std::endl;
        if (_lidar->when() > _last_actuation) memcpy(points, &(*_lidar), sizeof(points));
        for (int i = 0; i < vlp::VLP16_POINTS_PER_PACKET; i++) {
            IF_LIDAR_VIEWER(LidarViewer::add(points[i].x, points[i].y, points[i].z, points[i].i);)
        }
    }

   private:
    VLP16_LIDAR *_lidar;
    vlp::LIDAR_Point points[vlp::VLP16_POINTS_PER_PACKET];
    SmartData::Time _last_actuation;
};

int main(int argc, char *argv[])
{
    if (argc == 1) return EXIT_FAILURE;
    std::cout << "Thread::init()...\n";
    Thread::init();
    std::cout << "done." << std::endl;
    std::cout << "TSTP::init()...\n";
    TSTP::init();
    std::cout << "done." << std::endl;
    switch (atoi(argv[1])) {
        case 1: {
            std::cout << "Using IMU/GNSS hardware." << std::endl;
            IMU_Actuate imu(100000);
            break;
        }
        case 2: {
            std::cout << "Using Camera hardware." << std::endl;
            Camera_Actuate camera(100000);
            break;
        }
        case 3: {
            std::cout << "Using LIDAR hardware." << std::endl;
            LIDAR_Actuate lidar(100000);
            IF_LIDAR_VIEWER(LidarViewer::init(argc, argv);)
            break;
        }
    }
    while (1) {
        sleep(1000000000);
    };
    Thread::finish();
    return 0;
}

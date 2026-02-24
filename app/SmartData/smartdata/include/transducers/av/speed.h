#pragma once

#ifdef NO_DATA_SOURCE
    #include "fake_data.h"
#else
    #ifdef CARLA_V2_PROJECT
        #include "speed_socket_wrapper.h"
    #endif

    #ifdef GPS_HARDWARE
        #include "gps_carla_wrapper.h"
    #endif
#endif
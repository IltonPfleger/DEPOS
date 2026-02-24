#pragma once

#ifdef NO_DATA_SOURCE
    #include "fake_data.h"
#else
    #ifdef CARLA_V2_PROJECT
        #include "engine_rpm_carla_wrapper.h"
    #endif
#endif
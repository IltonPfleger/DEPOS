#pragma once

#ifdef NO_DATA_SOURCE
    #include "fake_data.h"
#else
    #ifdef ARTERY_PROJECT
        #include "co2_emission_artery_wrapper.h"
    #endif
#endif


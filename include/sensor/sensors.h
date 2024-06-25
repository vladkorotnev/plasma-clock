#include "../device_config.h"
#include "sensor.h"

#if HAS(TEMP_SENSOR)
#include "am2322.h"
#endif

#if HAS(LIGHT_SENSOR)
#include "light.h"
#endif

#if HAS(MOTION_SENSOR)
#include "motion.h"
#endif
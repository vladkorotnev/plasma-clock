#ifndef FEATUREFLAG_H_
#define FEATUREFLAG_H_

#define HAS(x) defined(HAS_##x)

#ifdef DEVICE_PLASMA_CLOCK
#include <devices/big_clock.h>
#endif

#endif
#pragma once
#include <device_config.h>

#if HAS(OUTPUT_MD_PLASMA)
#include <display/md_plasma.h>
#elif HAS(OUTPUT_WS0010)
#include <display/ws0010.h>
#elif HAS(OUTPUT_GU7000)
#include <display/gu7000.h>
#endif

#if HAS(OUTPUT_MD_PLASMA)
static MorioDenkiPlasmaDriver display_driver(
    HWCONF_PLASMA_DATABUS_GPIOS,
    HWCONF_PLASMA_CLK_GPIO,
    HWCONF_PLASMA_RESET_GPIO,
    HWCONF_PLASMA_BRIGHT_GPIO,
    HWCONF_PLASMA_SHOW_GPIO,
    HWCONF_PLASMA_HV_EN_GPIO
);
#elif HAS(OUTPUT_WS0010)
static Ws0010OledDriver display_driver(
    HWCONF_WS0010_DATABUS_GPIOS,
    HWCONF_WS0010_RS_GPIO,
    HWCONF_WS0010_EN_GPIO
);
#elif HAS(OUTPUT_GU7000)
static ItronGU7000Driver display_driver(
    HWCONF_GU7000_DATABUS_GPIOS,
    HWCONF_GU7000_WR_GPIO,
    HWCONF_GU7000_BUSY_GPIO
);
#else
#error Display module type not selected
#endif
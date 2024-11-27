#pragma once
#include <device_config.h>

#if HAS(OUTPUT_MD_PLASMA)
#include <display/md_plasma.h>
static MorioDenkiPlasmaDriver display_driver(
    HWCONF_PLASMA_DATABUS_GPIOS,
    HWCONF_PLASMA_CLK_GPIO,
    HWCONF_PLASMA_RESET_GPIO,
    HWCONF_PLASMA_BRIGHT_GPIO,
    HWCONF_PLASMA_SHOW_GPIO,
    HWCONF_PLASMA_HV_EN_GPIO
);
#elif HAS(OUTPUT_WS0010)
#include <display/ws0010.h>
static Ws0010OledDriver display_driver(
    HWCONF_WS0010_DATABUS_GPIOS,
    HWCONF_WS0010_RS_GPIO,
    HWCONF_WS0010_EN_GPIO
);
#elif HAS(OUTPUT_GU7000)
#include <display/gu7000.h>
static ItronGU7000Driver display_driver(
    HWCONF_GU7000_DATABUS_GPIOS,
    HWCONF_GU7000_WR_GPIO,
    HWCONF_GU7000_BUSY_GPIO
);
#elif HAS(OUTPUT_AKIZUKI_K875)
#include <display/akizuki_k875.h>
static AkizukiK875Driver display_driver(
    HWCONF_K875_LATCH_GPIO,
    HWCONF_K875_CLOCK_GPIO,
    HWCONF_K875_STROBE_GPIO,
    HWCONF_K875_SIN1_GPIO,
    HWCONF_K875_SIN2_GPIO,
    HWCONF_K875_SIN3_GPIO,
    HWCONF_K875_SACRIFICIAL_GPIO
);
#elif HAS(OUTPUT_GU312)
#include <display/gu312.h>
static NoritakeGu312Driver display_driver(
    HWCONF_GU312_DATABUS_GPIOS,
    HWCONF_GU312_CD_GPIO,
    HWCONF_GU312_WR_GPIO,
    HWCONF_GU312_BLANKING_GPIO,
    HWCONF_GU312_FEP_GPIO
);
#else
#error Display module type not selected
#endif
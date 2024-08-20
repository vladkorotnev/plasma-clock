#pragma once
#include <graphics/sprite.h>
#include <service/owm/weather.h>

const ani_sprite_t * sprite_from_conditions(weather_condition_t conditions);

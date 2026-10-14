#pragma once

#include <cstdint>

static constexpr int64_t COUNTDOWN_US_PER_SECOND = 1000000;

inline int countdown_seconds_remaining(int64_t deadlineUs, int64_t nowUs) {
    if(nowUs >= deadlineUs) return 0;

    int64_t remainingUs = deadlineUs - nowUs;
    return static_cast<int>((remainingUs + COUNTDOWN_US_PER_SECOND - 1) / COUNTDOWN_US_PER_SECOND);
}

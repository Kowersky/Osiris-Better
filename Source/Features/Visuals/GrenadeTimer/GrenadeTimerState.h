#pragma once

#include <array>
#include <CS2/Panorama/PanelHandle.h>

struct GrenadeTimerState {
    static constexpr int kMaxTimers = 8;
    cs2::PanelHandle containerPanelHandle;
    int activeCount{0};

    void reset() noexcept
    {
        containerPanelHandle = {};
        activeCount = 0;
    }
};

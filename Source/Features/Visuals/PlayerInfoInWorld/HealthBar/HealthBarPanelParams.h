#pragma once

#include <CS2/Panorama/CUILength.h>
#include <CS2/Panorama/StyleEnums.h>
#include <GameClient/Panorama/PanelAlignmentParams.h>
#include <GameClient/Panorama/PanelMarginParams.h>

namespace health_bar_panel_params
{
    namespace container_panel_params
    {
        static constexpr auto kAlignment = PanelAlignmentParams{
            .horizontalAlignment = cs2::k_EHorizontalAlignmentCenter,
            .verticalAlignment = cs2::k_EVerticalAlignmentTop
        };
        static constexpr auto kWidth = cs2::CUILength::pixels(80);
        static constexpr auto kHeight = cs2::CUILength::pixels(5);
        static constexpr auto kMargin = PanelMarginParams{.marginTop = cs2::CUILength::pixels(2)};
    }

    namespace bar_fill_params
    {
        static constexpr auto kHeight = cs2::CUILength::pixels(5);
    }
}

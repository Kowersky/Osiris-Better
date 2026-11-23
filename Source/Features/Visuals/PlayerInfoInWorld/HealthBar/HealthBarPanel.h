#pragma once

#include <algorithm>

#include <CS2/Classes/Color.h>
#include <CS2/Constants/ColorConstants.h>
#include <CS2/Panorama/CUILength.h>
#include <CS2/Panorama/CUIPanel.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoInWorldState.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoPanelCacheEntry.h>
#include <GameClient/Panorama/PanoramaUiPanel.h>
#include <Utils/ColorUtils.h>

template <typename HookContext>
class HealthBarPanel {
public:
    HealthBarPanel(HookContext& hookContext, cs2::CUIPanel* uiPanel, PlayerInfoPanelCacheEntry&) noexcept
        : hookContext{hookContext}
        , uiPanel{uiPanel}
    {
    }

    void update(auto&& playerPawn) const noexcept
    {
        if (!GET_CONFIG_VAR(player_info_vars::HealthBarEnabled)) {
            panel().setVisible(false);
            return;
        }

        panel().setVisible(true);

        const auto healthValue = playerPawn.health().valueOr(0);
        const auto healthFraction = std::clamp(healthValue, 0, 100) / 100.0f;

        auto&& barFill = panel().children()[0];
        barFill.setWidth(cs2::CUILength::percent(healthFraction * 100.0f));

        const auto barColor = color::HSBtoRGB(
            color::Hue{color::kRedHue + (color::kGreenHue - color::kRedHue) * healthFraction},
            color::Saturation{0.9f},
            color::Brightness{0.9f});
        barFill.setBackgroundColor(barColor);
    }

private:
    [[nodiscard]] decltype(auto) panel() const noexcept
    {
        return hookContext.template make<PanoramaUiPanel>(uiPanel);
    }

    HookContext& hookContext;
    cs2::CUIPanel* uiPanel;
};

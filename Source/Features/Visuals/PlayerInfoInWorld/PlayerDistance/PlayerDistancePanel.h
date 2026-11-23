#pragma once

#include <CS2/Classes/Color.h>
#include <CS2/Constants/ColorConstants.h>
#include <CS2/Panorama/CUIPanel.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoInWorldState.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoPanelCacheEntry.h>
#include <GameClient/Panorama/PanoramaLabel.h>
#include <GameClient/Panorama/PanoramaUiPanel.h>
#include <Utils/StringBuilder.h>

template <typename HookContext>
class PlayerDistancePanel {
public:
    PlayerDistancePanel(HookContext& hookContext, cs2::CUIPanel* uiPanel, PlayerInfoPanelCacheEntry&) noexcept
        : hookContext{hookContext}
        , uiPanel{uiPanel}
    {
    }

    void update(auto&& playerPawn) const noexcept
    {
        if (!GET_CONFIG_VAR(player_info_vars::PlayerDistanceEnabled)) {
            panel().setVisible(false);
            return;
        }

        panel().setVisible(true);

        const auto targetOrigin = playerPawn.absOrigin();
        if (!targetOrigin.hasValue())
            return;

        const auto localOrigin = hookContext.localPlayerController().pawn().template cast<PlayerPawn>().absOrigin();
        if (!localOrigin.hasValue())
            return;

        const auto dx = targetOrigin.value().x - localOrigin.value().x;
        const auto dy = targetOrigin.value().y - localOrigin.value().y;
        const auto dz = targetOrigin.value().z - localOrigin.value().z;
        const auto distSq = dx * dx + dy * dy + dz * dz;
        auto distanceInUnits = distSq;
        if (distSq > 0.0f) {
            auto guess = distSq * 0.5f;
            for (int i = 0; i < 5; ++i)
                guess = (guess + distSq / guess) * 0.5f;
            distanceInUnits = guess;
        }
        const auto distanceInMeters = static_cast<int>(distanceInUnits * 0.01905f);

        panel().clientPanel().template as<PanoramaLabel>().setText(
            StringBuilderStorage<16>{}.builder().put(distanceInMeters).put('m').cstring());
    }

private:
    [[nodiscard]] decltype(auto) panel() const noexcept
    {
        return hookContext.template make<PanoramaUiPanel>(uiPanel);
    }

    HookContext& hookContext;
    cs2::CUIPanel* uiPanel;
};

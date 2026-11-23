#pragma once

#include <CS2/Classes/Color.h>
#include <CS2/Panorama/CUILength.h>
#include <CS2/Panorama/CUIPanel.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoInWorldState.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoPanelCacheEntry.h>
#include <GameClient/Panorama/PanoramaUiPanel.h>

template <typename HookContext>
class EspBoxPanel {
public:
    EspBoxPanel(HookContext& hookContext, cs2::CUIPanel* uiPanel, PlayerInfoPanelCacheEntry&) noexcept
        : hookContext{hookContext}
        , uiPanel{uiPanel}
    {
    }

    void update(auto&& playerPawn) const noexcept
    {
        if (!GET_CONFIG_VAR(player_info_vars::EspBoxEnabled)) {
            panel().setVisible(false);
            return;
        }

        panel().setVisible(true);

        const auto isEnemy = playerPawn.isEnemy().value_or(true);
        const auto boxColor = isEnemy ? cs2::Color{255, 50, 50, 220} : cs2::Color{50, 255, 50, 220};
        panel().setBorder(cs2::CUILength::pixels(2), boxColor);
    }

private:
    [[nodiscard]] decltype(auto) panel() const noexcept
    {
        return hookContext.template make<PanoramaUiPanel>(uiPanel);
    }

    HookContext& hookContext;
    cs2::CUIPanel* uiPanel;
};

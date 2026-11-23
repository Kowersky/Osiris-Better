#pragma once

#include <CS2/Classes/Color.h>
#include <CS2/Constants/ColorConstants.h>
#include <CS2/Panorama/CUIPanel.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoInWorldState.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoPanelCacheEntry.h>
#include <GameClient/Panorama/PanoramaLabel.h>
#include <GameClient/Panorama/PanoramaUiPanel.h>

template <typename HookContext>
class PlayerNamePanel {
public:
    PlayerNamePanel(HookContext& hookContext, cs2::CUIPanel* uiPanel, PlayerInfoPanelCacheEntry&) noexcept
        : hookContext{hookContext}
        , uiPanel{uiPanel}
    {
    }

    void update(auto&& playerPawn) const noexcept
    {
        if (!GET_CONFIG_VAR(player_info_vars::PlayerNameEnabled)) {
            panel().setVisible(false);
            return;
        }

        panel().setVisible(true);

        const auto* name = playerPawn.getPlayerName();
        if (name)
            panel().clientPanel().template as<PanoramaLabel>().setText(name);
    }

private:
    [[nodiscard]] decltype(auto) panel() const noexcept
    {
        return hookContext.template make<PanoramaUiPanel>(uiPanel);
    }

    HookContext& hookContext;
    cs2::CUIPanel* uiPanel;
};

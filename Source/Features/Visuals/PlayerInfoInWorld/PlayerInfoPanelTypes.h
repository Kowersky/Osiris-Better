#pragma once

#include <tuple>

#include "ActiveWeaponAmmo/PlayerActiveWeaponAmmoPanel.h"
#include "EspBox/EspBoxPanel.h"
#include "HealthBar/HealthBarPanel.h"
#include "PlayerDistance/PlayerDistancePanel.h"
#include "PlayerHealth/PlayerHealthPanel.h"
#include "PlayerName/PlayerNamePanel.h"
#include "PlayerPositionArrow/PlayerPositionArrowPanel.h"
#include "PlayerStateIcons/PlayerStateIconsPanel.h"
#include "PlayerWeaponIcon/PlayerWeaponIconPanel.h"

template <typename HookContext>
using PlayerInfoPanelTypes = std::tuple<
    PlayerNamePanel<HookContext>,
    PlayerPositionArrowPanel<HookContext>,
    PlayerHealthPanel<HookContext>,
    HealthBarPanel<HookContext>,
    PlayerWeaponIconPanel<HookContext>,
    PlayerActiveWeaponAmmoPanel<HookContext>,
    PlayerStateIconsPanel<HookContext>,
    PlayerDistancePanel<HookContext>,
    EspBoxPanel<HookContext>>;

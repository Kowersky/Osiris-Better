#pragma once

#include <CS2/Classes/Entities/C_CSPlayerPawn.h>
#include <GameClient/Entities/BaseEntity.h>
#include <GameClient/Entities/PlayerPawn.h>
#include <GameClient/EntitySystem/EntitySystem.h>
#include <Features/Visuals/GrenadePrediction/GrenadePrediction.h>

template <typename HookContext>
class GrenadePredictor {
public:
    explicit GrenadePredictor(HookContext& hookContext) noexcept
        : hookContext{hookContext}
    {
    }

    void run() noexcept
    {
        hookContext.template make<EntitySystem>().forEachNetworkableEntityIdentity([this](const auto& entityIdentity) {
            const auto entityTypeInfo = hookContext.entityClassifier().classifyEntity(entityIdentity.entityClass);
            if (!entityTypeInfo.template is<cs2::C_CSPlayerPawn>())
                return;

            auto&& playerPawn = hookContext.template make<PlayerPawn>(static_cast<cs2::C_CSPlayerPawn*>(entityIdentity.entity));
            if (!playerPawn.isControlledByLocalPlayer())
                return;

            auto&& activeWeapon = playerPawn.getActiveWeapon();
            hookContext.template make<GrenadePrediction<HookContext>>().handleGrenadePrediction(playerPawn, activeWeapon);
        });
    }

    void onUnload() noexcept
    {
        hookContext.template make<GrenadePrediction<HookContext>>().clearPrediction();
    }

private:
    HookContext& hookContext;
};

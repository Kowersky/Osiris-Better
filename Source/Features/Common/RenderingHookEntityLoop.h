#pragma once

#include <CS2/Classes/Entities/C_BaseEntity.h>
#include <CS2/Classes/Entities/C_CSPlayerPawn.h>
#include <CS2/Classes/Entities/C_Inferno.h>
#include <GameClient/Entities/BaseEntity.h>
#include <GameClient/Entities/BaseModelEntity.h>
#include <GameClient/Entities/PlayerPawn.h>
#include <GameClient/Entities/SmokeGrenadeProjectile.h>
#include <Features/Visuals/ModelGlow/ModelGlow.h>
#include <Features/Visuals/OutlineGlow/OutlineGlow.h>
#include <Features/Visuals/PlayerInfoInWorld/PlayerInfoInWorld.h>
#include <GameClient/EntitySystem/EntitySystem.h>
#include <Features/Hud/BombPlantAlert/BombPlantAlert.h>
#include <Features/Visuals/GrenadeTimer/GrenadeTimer.h>
#include <Features/Visuals/RadarHack/RadarHackConfigVariables.h>

template <typename HookContext>
class RenderingHookEntityLoop {
public:
    explicit RenderingHookEntityLoop(HookContext& hookContext) noexcept
        : hookContext{hookContext}
    {
    }

    void run() const noexcept
    {
        auto bombPlantAlertVisibility = Visibility::Hidden;
        hookContext.template make<GrenadeTimer>().beginFrame();
        hookContext.template make<EntitySystem>().forEachNetworkableEntityIdentity([this, &bombPlantAlertVisibility](const auto& entityIdentity) { handleEntityIdentity(entityIdentity, bombPlantAlertVisibility); });
        hookContext.template make<GrenadeTimer>().endFrame();
        hookContext.template make<ModelGlow>().postUpdateInMainThread();
        if (bombPlantAlertVisibility == Visibility::Hidden)
            hookContext.template make<BombPlantAlert>().hide();
    }

private:
    void handleEntityIdentity(const cs2::CEntityIdentity& entityIdentity, Visibility& bombPlantAlertVisibility) const noexcept
    {
        const auto entityTypeInfo = hookContext.entityClassifier().classifyEntity(entityIdentity.entityClass);
        auto&& baseEntity = hookContext.template make<BaseEntity>(static_cast<cs2::C_BaseEntity*>(entityIdentity.entity));

        if (entityTypeInfo.template is<cs2::C_CSPlayerPawn>()) {
            auto&& playerPawn = baseEntity.template as<PlayerPawn>();
            if (playerPawn.isEnemy().value_or(false))
                playerPawn.forceSpotted();
            if (hookContext.config().template getVariable<radar_hack_vars::Enabled>() && !hookContext.config().template getVariable<radar_hack_vars::OnlyEnemies>() && !playerPawn.isEnemy().value_or(false))
                playerPawn.forceSpotted();
            hookContext.template make<PlayerInfoInWorld>().drawPlayerInformation(playerPawn);
            updateModelGlow<PlayerModelGlow>(playerPawn, entityTypeInfo);
            applyOutlineGlow<PlayerOutlineGlow>(playerPawn, entityTypeInfo);
            if (bombPlantAlertVisibility != Visibility::Visible)
                bombPlantAlertVisibility = hookContext.template make<BombPlantAlert>().show(playerPawn);
        } else if (entityTypeInfo.template is<cs2::C_C4>()) {
            if (hookContext.config().template getVariable<radar_hack_vars::Enabled>() && hookContext.config().template getVariable<radar_hack_vars::ShowBomb>())
                forceEntitySpotted(entityIdentity.entity);
            updateModelGlow<DroppedBombModelGlow>(baseEntity.template as<BaseWeapon>(), entityTypeInfo);
            applyOutlineGlow<DroppedBombOutlineGlow>(baseEntity, entityTypeInfo);
        } else if (entityTypeInfo.template is<cs2::CBaseAnimGraph>()) {
            updateModelGlow<DefuseKitModelGlow>(baseEntity, entityTypeInfo);
            applyOutlineGlow<DefuseKitOutlineGlow>(baseEntity, entityTypeInfo);
        } else if (entityTypeInfo.template is<cs2::CPlantedC4>()) {
            updateModelGlow<TickingBombModelGlow>(baseEntity.template as<PlantedC4>(), entityTypeInfo);
            applyOutlineGlow<TickingBombOutlineGlow>(baseEntity.template as<PlantedC4>(), entityTypeInfo);
        }  else if (entityTypeInfo.template is<cs2::C_Hostage>()) {
            applyOutlineGlow<HostageOutlineGlow>(baseEntity, entityTypeInfo);
        } else if (entityTypeInfo.isGrenadeProjectile()) {
            if (entityTypeInfo.template is<cs2::C_SmokeGrenadeProjectile>())
                hookContext.template make<GrenadeTimer>().showSmokeTimer(baseEntity.template as<SmokeGrenadeProjectile>());
            updateModelGlow<GrenadeProjectileModelGlow>(baseEntity, entityTypeInfo);
            applyOutlineGlow<GrenadeProjectileOutlineGlow>(baseEntity, entityTypeInfo);
        } else if (entityTypeInfo.template is<cs2::C_Inferno>()) {
            hookContext.template make<GrenadeTimer>().showMolotovTimer(baseEntity);
        } else if (entityTypeInfo.isWeapon()) {
            updateModelGlow<WeaponModelGlow>(baseEntity.template as<BaseWeapon>(), entityTypeInfo);
            applyOutlineGlow<WeaponOutlineGlow>(baseEntity, entityTypeInfo);
        }
    }

    static void forceEntitySpotted(void* entity) noexcept
    {
        if (!entity)
            return;
        constexpr auto kEntitySpottedStateOffset = 0x1EB8;
        constexpr auto kSpottedOffset = 0x8;
        auto* spotted = reinterpret_cast<bool*>(
            reinterpret_cast<std::byte*>(entity) + kEntitySpottedStateOffset + kSpottedOffset);
        *spotted = true;
    }

    template <template <typename> typename Glow, typename... Args>
    void updateModelGlow(Args&&... args) const
    {
        hookContext.template make<ModelGlow>().updateInMainThread()(Glow{hookContext}, std::forward<Args>(args)...);
    }

    template <template <typename> typename Glow, typename... Args>
    void applyOutlineGlow(Args&&... args) const
    {
        hookContext.template make<OutlineGlow>().applyGlow()(Glow{hookContext}, std::forward<Args>(args)...);
    }

    HookContext& hookContext;
};

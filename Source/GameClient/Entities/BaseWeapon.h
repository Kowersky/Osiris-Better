#pragma once

#include <cstdint>

#include <CS2/Classes/Entities/C_CSWeaponBase.h>
#include <CS2/Classes/Entities/WeaponEntities.h>
#include <CS2/Classes/CCSWeaponBaseVData.h>
#include <Features/Visuals/GrenadePrediction/GrenadeKind.h>
#include <MemoryPatterns/PatternTypes/WeaponPatternTypes.h>
#include <MemoryPatterns/PatternTypes/WeaponVDataPatternTypes.h>
#include "BaseEntity.h"
#include "EntityClassifier.h"
#include "WeaponVData.h"

template <typename HookContext>
class BaseWeapon {
public:
    BaseWeapon(HookContext& hookContext, cs2::C_CSWeaponBase* baseWeapon) noexcept
        : hookContext{hookContext}
        , baseWeapon{baseWeapon}
    {
    }

    using RawType = cs2::C_CSWeaponBase;

    template <template <typename...> typename EntityType>
    [[nodiscard]] bool is() const noexcept
    {
        return baseEntity().template is<EntityType>();
    }

    template <template <typename...> typename EntityType>
    [[nodiscard]] decltype(auto) cast() const noexcept
    {
        return baseEntity().template cast<EntityType>();
    }

    [[nodiscard]] decltype(auto) baseEntity() const noexcept
    {
        return hookContext.template make<BaseEntity>(baseWeapon);
    }

    [[nodiscard]] bool isSniperRifle() const noexcept
    {
        switch (baseEntity().classify().typeIndex) {
        case EntityTypeInfo::indexOf<cs2::C_WeaponSSG08>():
        case EntityTypeInfo::indexOf<cs2::C_WeaponAWP>():
        case EntityTypeInfo::indexOf<cs2::C_WeaponG3SG1>():
        case EntityTypeInfo::indexOf<cs2::C_WeaponSCAR20>(): return true;
        default: return false;
        }
    }

    [[nodiscard]] auto bulletInaccuracy() const noexcept
    {
        return inaccuracy() + spread();
    }

    [[nodiscard]] auto getName() const noexcept
    {
        return vData().name().valueOr(nullptr);
    }

    [[nodiscard]] Optional<cs2::GrenadeKind> grenadeKind() const noexcept
    {
        const auto typeInfo = baseEntity().classify();
        if (typeInfo.template is<cs2::C_Flashbang>()) return cs2::GrenadeKind::Flashbang;
        if (typeInfo.template is<cs2::C_HEGrenade>()) return cs2::GrenadeKind::HEGrenade;
        if (typeInfo.template is<cs2::C_SmokeGrenade>()) return cs2::GrenadeKind::SmokeGrenade;
        if (typeInfo.template is<cs2::C_MolotovGrenade>()) return cs2::GrenadeKind::Molotov;
        if (typeInfo.template is<cs2::C_IncendiaryGrenade>()) return cs2::GrenadeKind::Incendiary;
        if (typeInfo.template is<cs2::C_DecoyGrenade>()) return cs2::GrenadeKind::Decoy;
        return {};
    }

    [[nodiscard]] auto clipAmmo() const noexcept
    {
        return hookContext.patternSearchResults().template get<OffsetToClipAmmo>().of(baseWeapon).toOptional();
    }

    [[nodiscard]] auto getSceneObjectUpdater() const noexcept
    {
        return reinterpret_cast<std::uint64_t(*)(cs2::C_CSWeaponBase*, void*, bool)>(sceneObjectUpdaterHandle() ? sceneObjectUpdaterHandle()->updaterFunction : nullptr);
    }

    void setSceneObjectUpdater(auto x) const noexcept
    {
        if (sceneObjectUpdaterHandle())
            sceneObjectUpdaterHandle()->updaterFunction = reinterpret_cast<std::uint64_t(*)(void*, void*, bool)>(x);
    }

private:
    [[nodiscard]] Optional<float> inaccuracy() const noexcept
    {
        const auto getInaccuracyFn = hookContext.patternSearchResults().template get<PointerToGetInaccuracyFunction>();
        if (baseWeapon && getInaccuracyFn)
            return getInaccuracyFn(baseWeapon, nullptr, nullptr);
        return {};
    }

    [[nodiscard]] Optional<float> spread() const
    {
        return vData().spread()[spreadIndex()];
    }

    [[nodiscard]] auto sceneObjectUpdaterHandle() const noexcept
    {
        return hookContext.patternSearchResults().template get<OffsetToWeaponSceneObjectUpdaterHandle>().of(baseWeapon).valueOr(nullptr);
    }

    [[nodiscard]] auto spreadIndex() const
    {
        using enum cs2::CSWeaponMode;
        return toUnderlying(weaponMode() == Secondary ? Secondary : Primary);
    }

    [[nodiscard]] auto weaponMode() const
    {
        return hookContext.patternSearchResults().template get<OffsetToWeaponMode>().of(baseWeapon).toOptional();
    }

    [[nodiscard]] auto vData() const
    {
        return hookContext.template make<WeaponVData>(static_cast<cs2::CCSWeaponBaseVData*>(hookContext.template make<BaseEntity>(baseWeapon).vData().valueOr(nullptr)));
    }

    HookContext& hookContext;
    cs2::C_CSWeaponBase* baseWeapon;
};

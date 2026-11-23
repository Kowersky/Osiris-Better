#pragma once

#include <cstddef>
#include <cstdint>

#include <CS2/Classes/Entities/GrenadeProjectiles.h>
#include <GameClient/Entities/BaseEntity.h>
#include <MemoryPatterns/PatternTypes/SmokeGrenadeProjectilePatternTypes.h>
#include <Utils/Optional.h>

template <typename HookContext>
class SmokeGrenadeProjectile {
public:
    SmokeGrenadeProjectile(HookContext& hookContext, cs2::C_SmokeGrenadeProjectile* smokeGrenadeProjectile) noexcept
        : hookContext{hookContext}
        , smokeGrenadeProjectile{smokeGrenadeProjectile}
    {
    }

    using RawType = cs2::C_SmokeGrenadeProjectile;

    [[nodiscard]] decltype(auto) baseEntity() const noexcept
    {
        return hookContext.template make<BaseEntity>(static_cast<cs2::C_BaseEntity*>(smokeGrenadeProjectile));
    }

    [[nodiscard]] auto didSmokeEffect() const noexcept
    {
        return hookContext.patternSearchResults().template get<OffsetToDidSmokeEffect>().of(smokeGrenadeProjectile).toOptional();
    }

    [[nodiscard]] Optional<std::int32_t> smokeEffectTickBegin() const noexcept
    {
        auto* didSmokeEffectPtr = hookContext.patternSearchResults().template get<OffsetToDidSmokeEffect>().of(smokeGrenadeProjectile).get();
        if (didSmokeEffectPtr) {
            return *reinterpret_cast<const std::int32_t*>(reinterpret_cast<const std::byte*>(didSmokeEffectPtr) - sizeof(std::int32_t));
        }
        return {};
    }

private:
    HookContext& hookContext;
    cs2::C_SmokeGrenadeProjectile* smokeGrenadeProjectile;
};

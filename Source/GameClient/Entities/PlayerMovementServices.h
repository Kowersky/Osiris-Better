#pragma once

#include <cstddef>
#include <CS2/Classes/Entities/C_CSPlayerPawn.h>
#include <CS2/Classes/Vector.h>
#include <MemoryPatterns/PatternTypes/PlayerPawnPatternTypes.h>
#include <Features/Visuals/GrenadePrediction/GrenadePredictionOffsets.h>
#include <Utils/Optional.h>

template <typename HookContext>
class PlayerMovementServices {
public:
    PlayerMovementServices(HookContext& hookContext, cs2::C_CSPlayerPawn* playerPawn) noexcept
        : hookContext{hookContext}
        , playerPawn{playerPawn}
        , movementServices{static_cast<cs2::CCSPlayer_MovementServices*>(hookContext.patternSearchResults().template get<OffsetToMovementServices>().of(playerPawn).valueOr(nullptr))}
    {
    }

    [[nodiscard]] auto stashGrenadeParameterWhen() const noexcept
    {
        return readField<float>(movementServices, grenade_prediction_offsets::m_fStashGrenadeParameterWhen);
    }

    [[nodiscard]] auto grenadeParametersStashed() const noexcept
    {
        return readField<bool>(playerPawn, grenade_prediction_offsets::m_bGrenadeParametersStashed);
    }

    [[nodiscard]] auto stashedShootAngles() const noexcept
    {
        return readField<cs2::Vector>(playerPawn, grenade_prediction_offsets::m_angStashedShootAngles);
    }

    [[nodiscard]] auto stashedGrenadeThrowPosition() const noexcept
    {
        return readField<cs2::Vector>(playerPawn, grenade_prediction_offsets::m_vecStashedGrenadeThrowPosition);
    }

    [[nodiscard]] auto stashedVelocity() const noexcept
    {
        return readField<cs2::Vector>(playerPawn, grenade_prediction_offsets::m_vecStashedVelocity);
    }

private:
    template <typename T>
    [[nodiscard]] Optional<T> readField(void* base, std::int32_t offset) const noexcept
    {
        if (!base || offset <= 0)
            return {};
        auto* ptr = reinterpret_cast<T*>(reinterpret_cast<std::byte*>(base) + offset);
        return Optional<T>{*ptr};
    }

    HookContext& hookContext;
    cs2::C_CSPlayerPawn* playerPawn;
    cs2::CCSPlayer_MovementServices* movementServices;
};

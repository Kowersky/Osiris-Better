#pragma once

// Offsets from cs2-dumper (build 14200, 2026-11-23)
// Update these when CS2 updates by running cs2-dumper and checking output/client_dll.rs

#include <cstdint>

namespace grenade_prediction_offsets
{
    // C_BaseEntity
    constexpr std::int32_t m_vecAbsVelocity = 0x3F8;
    constexpr std::int32_t m_vecViewOffset = 0xF60;

    // CCSPlayer_MovementServices
    constexpr std::int32_t m_fStashGrenadeParameterWhen = 0x684;

    // C_CSPlayerPawn
    constexpr std::int32_t m_grenadeParameterStashTime = 0x1F50;
    constexpr std::int32_t m_bGrenadeParametersStashed = 0x1F54;
    constexpr std::int32_t m_angStashedShootAngles = 0x1F58;
    constexpr std::int32_t m_vecStashedGrenadeThrowPosition = 0x1F64;
    constexpr std::int32_t m_vecStashedVelocity = 0x1F7C;

    // C_BaseCSGrenade
    constexpr std::int32_t m_bPinPulled = 0x1F63;
    constexpr std::int32_t m_fThrowTime = 0x1F68;
    constexpr std::int32_t m_flThrowStrength = 0x1F70;
}

#pragma once

#include <algorithm>
#include <cstdint>

#include <CS2/Classes/Vector.h>
#include <GameClient/Panorama/PanelFontParams.h>
#include <GameClient/Panorama/PanelHandle.h>
#include <GameClient/Panorama/PanoramaLabel.h>
#include <GameClient/Panorama/PanoramaTransformations.h>
#include <GameClient/Panorama/PanoramaUiEngine.h>
#include <GameClient/Panorama/PanoramaUiPanel.h>
#include <GameClient/WorldToScreen/ViewToProjectionMatrix.h>
#include <GameClient/WorldToScreen/WorldToClipSpaceConverter.h>
#include <HookContext/HookContextMacros.h>
#include <Utils/ColorUtils.h>
#include <Utils/StringBuilder.h>

#include "GrenadeTimerConfigVariables.h"
#include "GrenadeTimerState.h"

template <typename HookContext>
class GrenadeTimer {
public:
    explicit GrenadeTimer(HookContext& hookContext) noexcept
        : hookContext{hookContext}
    {
    }

    void beginFrame() const noexcept
    {
        state().activeCount = 0;
    }

    void showSmokeTimer(void* entity) const noexcept
    {
        if (!hookContext.config().template getVariable<grenade_timer_vars::SmokeTimerEnabled>())
            return;

        constexpr auto kSmokeEffectTickBeginOffset = 0x1360;
        constexpr auto kDidSmokeEffectOffset = 0x1364;
        constexpr auto kSmokeDuration = 18.0f;
        constexpr auto kTickInterval = 1.0f / 64.0f;

        if (!entity) return;

        const auto didSmoke = *reinterpret_cast<const bool*>(
            reinterpret_cast<const std::byte*>(entity) + kDidSmokeEffectOffset);
        if (!didSmoke) return;

        const auto tickBegin = *reinterpret_cast<const std::int32_t*>(
            reinterpret_cast<const std::byte*>(entity) + kSmokeEffectTickBeginOffset);
        if (tickBegin <= 0) return;

        const auto curtime = hookContext.globalVars().curtime();
        if (!curtime.hasValue()) return;

        const auto smokeStartTime = static_cast<float>(tickBegin) * kTickInterval;
        const auto elapsed = curtime.value() - smokeStartTime;
        const auto remaining = kSmokeDuration - elapsed;
        if (remaining <= 0.0f) return;

        showTimerAtEntity(entity, remaining, kSmokeDuration);
    }

    void showMolotovTimer(void* entity) const noexcept
    {
        if (!hookContext.config().template getVariable<grenade_timer_vars::MolotovTimerEnabled>())
            return;

        constexpr auto kFireLifetimeOffset = 0x1A50;
        constexpr auto kFireEffectTickBeginOffset = 0x1A5C;
        constexpr auto kTickInterval = 1.0f / 64.0f;

        if (!entity) return;

        const auto fireLifetime = *reinterpret_cast<const float*>(
            reinterpret_cast<const std::byte*>(entity) + kFireLifetimeOffset);
        if (fireLifetime <= 0.0f) return;

        const auto tickBegin = *reinterpret_cast<const std::int32_t*>(
            reinterpret_cast<const std::byte*>(entity) + kFireEffectTickBeginOffset);
        if (tickBegin <= 0) return;

        const auto curtime = hookContext.globalVars().curtime();
        if (!curtime.hasValue()) return;

        const auto startTime = static_cast<float>(tickBegin) * kTickInterval;
        const auto elapsed = curtime.value() - startTime;
        const auto remaining = fireLifetime - elapsed;
        if (remaining <= 0.0f) return;

        showTimerAtEntity(entity, remaining, fireLifetime);
    }

    void endFrame() const noexcept
    {
        auto&& container = getContainerPanel();
        if (!container)
            return;

        auto&& children = container.children();
        for (int i = state().activeCount; ; ++i) {
            auto&& child = children[i];
            if (!child)
                break;
            child.setVisible(false);
        }
    }

    void onUnload() const noexcept
    {
        hookContext.template make<PanoramaUiEngine>().deletePanelByHandle(state().containerPanelHandle);
    }

private:
    void showTimerAtEntity(void* entity, float remaining, float totalDuration) const noexcept
    {
        constexpr auto kAbsOriginOffset = 0x80;

        const auto* origin = reinterpret_cast<const cs2::Vector*>(
            reinterpret_cast<const std::byte*>(entity) + kAbsOriginOffset);

        const auto positionInClipSpace = hookContext.template make<WorldToClipSpaceConverter>().toClipSpace(*origin);
        if (!positionInClipSpace.onScreen())
            return;

        auto&& panel = getOrCreateTimerPanel();
        if (!panel)
            return;

        panel.setVisible(true);
        panel.setZIndex(-positionInClipSpace.z);

        const auto fraction = remaining / totalDuration;
        const auto timerColor = color::HSBtoRGB(
            color::Hue{color::kRedHue + (color::kGreenHue - color::kRedHue) * fraction},
            color::Saturation{0.9f},
            color::Brightness{1.0f});
        panel.setColor(timerColor);

        StringBuilderStorage<10> storage;
        auto builder = storage.builder();
        builder.put(static_cast<int>(remaining), '.', static_cast<int>(remaining * 10) % 10, 's');
        panel.clientPanel().template as<PanoramaLabel>().setText(builder.cstring());

        const auto fovScale = ViewToProjectionMatrix{hookContext}.getFovScale();
        const auto scale = std::clamp(500.0f / (positionInClipSpace.z / fovScale + 400.0f), 0.3f, 1.0f);
        const auto deviceCoordinates = positionInClipSpace.toNormalizedDeviceCoordinates();

        PanoramaTransformations{
            hookContext.panoramaTransformFactory().scale(scale),
            hookContext.panoramaTransformFactory().translate(deviceCoordinates.getX(), deviceCoordinates.getY())
        }.applyTo(panel);

        state().activeCount++;
    }

    [[nodiscard]] decltype(auto) getOrCreateTimerPanel() const noexcept
    {
        auto&& container = getOrCreateContainerPanel();
        if (!container)
            return hookContext.template make<PanoramaUiPanel>(nullptr);

        auto&& child = container.children()[state().activeCount];
        if (child)
            return utils::lvalue<decltype(child)>(child);

        auto&& label = hookContext.panelFactory().createLabelPanel(container).uiPanel();
        label.setFont(PanelFontParams{
            .fontFamily = "Stratum2 Bold TF, 'Arial Unicode MS'",
            .fontSize = 24});
        label.setAlign(PanelAlignmentParams{
            .horizontalAlignment = cs2::k_EHorizontalAlignmentCenter,
            .verticalAlignment = cs2::k_EVerticalAlignmentCenter});
        label.setTextShadow(PanelShadowParams{
            .horizontalOffset = cs2::CUILength::pixels(0),
            .verticalOffset = cs2::CUILength::pixels(0),
            .blurRadius = cs2::CUILength::pixels(3),
            .strength = 3,
            .color = cs2::Color{0, 0, 0, 255}});
        return utils::lvalue<decltype(label)>(label);
    }

    [[nodiscard]] decltype(auto) getOrCreateContainerPanel() const noexcept
    {
        return hookContext.template make<PanelHandle>(state().containerPanelHandle).getOrInit([this]() -> decltype(auto) {
            auto&& panel = hookContext.panelFactory().createPanel(hookContext.hud().getHudReticle()).uiPanel();
            panel.fitParent();
            return utils::lvalue<decltype(panel)>(panel);
        });
    }

    [[nodiscard]] decltype(auto) getContainerPanel() const noexcept
    {
        return hookContext.template make<PanelHandle>(state().containerPanelHandle).get();
    }

    [[nodiscard]] auto& state() const noexcept
    {
        return hookContext.featuresStates().visualFeaturesStates.grenadeTimerState;
    }

    HookContext& hookContext;
};

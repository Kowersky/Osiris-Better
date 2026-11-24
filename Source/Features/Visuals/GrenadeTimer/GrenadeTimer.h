#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <CS2/Classes/Entities/C_BaseEntity.h>
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

    template <typename SmokeProjectile>
    void showSmokeTimer(SmokeProjectile&& smokeProjectile) const noexcept
    {
        if (!hookContext.config().template getVariable<grenade_timer_vars::SmokeTimerEnabled>())
            return;

        const auto didSmoke = smokeProjectile.didSmokeEffect().valueOr(false);
        if (!didSmoke)
            return;

        constexpr auto kSmokeDuration = 18.0f;
        constexpr auto kTickInterval = 1.0f / 64.0f;

        const auto curtime = hookContext.globalVars().curtime();
        if (!curtime.hasValue())
            return;

        const auto tickBegin = smokeProjectile.smokeEffectTickBegin();
        if (!tickBegin.hasValue())
            return;

        const auto origin = smokeProjectile.baseEntity().absOrigin();
        if (!origin.hasValue())
            return;

        const auto startTime = static_cast<float>(tickBegin.value()) * kTickInterval;
        const auto remaining = kSmokeDuration - (curtime.value() - startTime);
        if (remaining <= 0.0f)
            return;

        auto smokeCenter = origin.value();
        smokeCenter.z += 50.0f;
        showTimerAtPosition(smokeCenter, remaining, kSmokeDuration);
    }

    template <typename BaseEntityType>
    void showMolotovTimer(BaseEntityType&& baseEntity) const noexcept
    {
        if (!hookContext.config().template getVariable<grenade_timer_vars::MolotovTimerEnabled>())
            return;

        constexpr auto kTickInterval = 1.0f / 64.0f;
        constexpr auto kOffsetFireLifetime = 0x1A50;
        constexpr auto kOffsetFireEffectTickBegin = 0x1A5C;

        const auto curtime = hookContext.globalVars().curtime();
        if (!curtime.hasValue())
            return;

        const auto origin = baseEntity.absOrigin();
        if (!origin.hasValue())
            return;

        auto* entityPtr = static_cast<cs2::C_BaseEntity*>(baseEntity);
        if (!entityPtr)
            return;

        const auto fireLifetime = *reinterpret_cast<const float*>(reinterpret_cast<const std::byte*>(entityPtr) + kOffsetFireLifetime);
        const auto fireTickBegin = *reinterpret_cast<const std::int32_t*>(reinterpret_cast<const std::byte*>(entityPtr) + kOffsetFireEffectTickBegin);

        if (fireLifetime <= 0.0f || fireTickBegin <= 0)
            return;

        const auto startTime = static_cast<float>(fireTickBegin) * kTickInterval;
        const auto remaining = fireLifetime - (curtime.value() - startTime);
        if (remaining <= 0.0f)
            return;

        showTimerAtPosition(origin.value(), remaining, fireLifetime);
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
    void showTimerAtPosition(const cs2::Vector& origin, float remaining, float totalDuration) const noexcept
    {
        const auto positionInClipSpace = hookContext.template make<WorldToClipSpaceConverter>().toClipSpace(origin);
        if (!positionInClipSpace.onScreen())
            return;

        auto&& panel = getOrCreateTimerPanel();
        if (!panel)
            return;

        panel.setVisible(true);
        panel.setZIndex(-positionInClipSpace.z);

        const auto fraction = std::clamp(remaining / totalDuration, 0.0f, 1.0f);
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
            hookContext.panoramaTransformFactory().translate(deviceCoordinates.getX(), deviceCoordinates.getY()),
            hookContext.panoramaTransformFactory().translate(cs2::CUILength::percent(-50), cs2::CUILength::percent(-50))
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

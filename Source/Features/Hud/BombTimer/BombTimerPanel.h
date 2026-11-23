#pragma once

#include <algorithm>

#include <CS2/Constants/BombsiteIndex.h>
#include <CS2/Constants/IconURLs.h>
#include <GameClient/Panorama/ImagePanel.h>
#include <GameClient/Panorama/PanelHandle.h>
#include <GameClient/Panorama/PanoramaLabel.h>
#include <GameClient/Panorama/PanoramaUiEngine.h>
#include <Utils/ColorUtils.h>
#include <Utils/StringBuilder.h>
#include "BombTimerPanelFactory.h"

template <typename HookContext>
class BombTimerPanel {
public:
    explicit BombTimerPanel(HookContext& hookContext) noexcept
        : hookContext{hookContext}
    {
    }

    void showAndUpdate(auto&& tickingC4) const
    {
        auto&& container = bombTimerContainerPanel();
        container.show();

        setBombsiteIcon(container.children()[0].clientPanel().template as<ImagePanel>(), getBombsiteIconUrl(tickingC4));
        setTimeToExplosion(container.children()[1].clientPanel().template as<PanoramaLabel>(), tickingC4.getTimeToExplosion().valueOr(0.0f));
        updateSafetyDistance(container.children()[2], tickingC4);
    }

    void hide() const
    {
        uiEngine().getPanelFromHandle(state().bombTimerContainerPanelHandle).hide();
    }

private:
    void updateSafetyDistance(auto&& safetyPanel, auto&& tickingC4) const
    {
        const auto bombOrigin = tickingC4.baseEntity().absOrigin();
        if (!bombOrigin.hasValue()) {
            safetyPanel.setVisible(false);
            return;
        }

        const auto localOrigin = hookContext.localPlayerController().pawn().absOrigin();
        if (!localOrigin.hasValue()) {
            safetyPanel.setVisible(false);
            return;
        }

        safetyPanel.setVisible(true);

        const auto dx = bombOrigin.value().x - localOrigin.value().x;
        const auto dy = bombOrigin.value().y - localOrigin.value().y;
        const auto dz = bombOrigin.value().z - localOrigin.value().z;
        const auto distSq = dx * dx + dy * dy + dz * dz;
        auto dist = distSq;
        if (distSq > 0.0f) {
            auto guess = distSq * 0.5f;
            for (int i = 0; i < 5; ++i)
                guess = (guess + distSq / guess) * 0.5f;
            dist = guess;
        }

        constexpr auto kSafeDistance = 1750.0f;
        constexpr auto kLethalDistance = 500.0f;
        const auto safetyFraction = std::clamp((dist - kLethalDistance) / (kSafeDistance - kLethalDistance), 0.0f, 1.0f);

        const auto safetyColor = color::HSBtoRGB(
            color::Hue{color::kRedHue + (color::kGreenHue - color::kRedHue) * safetyFraction},
            color::Saturation{0.9f},
            color::Brightness{1.0f});
        safetyPanel.setColor(safetyColor);

        const auto distMeters = static_cast<int>(dist * 0.01905f);
        StringBuilderStorage<20> storage;
        auto builder = storage.builder();
        if (safetyFraction >= 0.9f)
            builder.put("SAFE ", distMeters, 'm');
        else if (safetyFraction >= 0.4f)
            builder.put("RYZYKO ", distMeters, 'm');
        else
            builder.put("UCIEKAJ ", distMeters, 'm');
        safetyPanel.clientPanel().template as<PanoramaLabel>().setText(builder.cstring());
    }

    void setBombsiteIcon(auto&& bombsiteIconPanel, const char* bombsiteIconUrl) const
    {
        if (bombsiteIconUrl)
            bombsiteIconPanel.setImageSvg(bombsiteIconUrl);
    }

    void setTimeToExplosion(auto&& bombTimerTextPanel, float timeToExplosion) const
    {
        StringBuilderStorage<10> storage;
        StringBuilder builder = storage.builder();
        builder.put(static_cast<int>(timeToExplosion), '.', static_cast<int>(timeToExplosion * 10) % 10);
        bombTimerTextPanel.setText(builder.cstring());
    }

    [[nodiscard]] const char* getBombsiteIconUrl(auto&& tickingC4) const
    {
        auto&& bombsiteIndex = tickingC4.bombsiteIndex();
        if (!bombsiteIndex.hasValue())
            return nullptr;

        switch (bombsiteIndex.value()) {
        case cs2::BombsiteIndex::BombsiteA: return cs2::kBombSiteAIconUrl;
        case cs2::BombsiteIndex::BombsiteB: return cs2::kBombSiteBIconUrl;
        default: return nullptr;
        }
    }

    [[nodiscard]] auto& state() const
    {
        return hookContext.featuresStates().hudFeaturesStates.bombTimerState;
    }

    [[nodiscard]] decltype(auto) bombTimerContainerPanel() const
    {
        return hookContext.template make<PanelHandle>(state().bombTimerContainerPanelHandle).getOrInit(createContainerPanel());
    }

    [[nodiscard]] auto createContainerPanel() const noexcept
    {
        return [this] () -> decltype(auto) {
            auto&& factory = hookContext.template make<BombTimerPanelFactory>();
            auto&& container = factory.createContainerPanel(hookContext.hud().scoreAndTimeAndBomb());
            factory.createBombsiteIconPanel(container);
            factory.createTimerTextPanel(container);
            factory.createSafetyDistancePanel(container);
            return utils::lvalue<decltype(container)>(container);
        };
    }

    [[nodiscard]] decltype(auto) uiEngine() const
    {
        return hookContext.template make<PanoramaUiEngine>();
    }

    HookContext& hookContext;
};

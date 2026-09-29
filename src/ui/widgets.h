#pragma once

#include <functional>
#include <string>
#include <utility>

#include "theme.h"

namespace app::ui {

inline float clamp01(float value) {
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

// One pixel divider.
inline void hairline(eui::Ui& ui, const std::string& id, float x, float y,
                     float width, const eui::Color& color) {
    ui.rect(id).position(x, y).size(width, 1.0f).color(color).build();
}

struct IconButtonStyle {
    float size = 34.0f;
    float radius = 10.0f;
    float iconSize = 15.0f;
    eui::Color background{0.0f, 0.0f, 0.0f, 0.0f};
    eui::Color hover{0.0f, 0.0f, 0.0f, 0.0f};
    eui::Color pressed{0.0f, 0.0f, 0.0f, 0.0f};
    eui::Color tint{1.0f, 1.0f, 1.0f, 1.0f};
    bool bordered = false;
};

// Round-cornered icon button drawn from primitives so it can sit anywhere.
inline void iconButton(eui::Ui& ui, const std::string& id, unsigned int icon,
                       float x, float y, const IconButtonStyle& style,
                       std::function<void()> onClick) {
    const Palette& p = palette();
    const eui::Color hover =
        style.hover.a > 0.0f ? style.hover : alpha(p.surfaceHover, 0.9f);
    const eui::Color pressed =
        style.pressed.a > 0.0f ? style.pressed : alpha(p.surfaceActive, 0.95f);

    ui.rect(id)
        .position(x, y)
        .size(style.size, style.size)
        .radius(style.radius)
        .border(style.bordered ? 1.0f : 0.0f,
                style.bordered ? alpha(p.border, 0.95f) : p.background)
        .states(style.background, hover, pressed)
        .transition(uiTransition())
        .animate(eui::AnimProperty::Color)
        .onClick(std::move(onClick))
        .build();

    ui.text(id + ".icon")
        .icon(icon)
        .position(x, y)
        .size(style.size, style.size)
        .fontSize(style.iconSize)
        .color(style.tint)
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

// Themed pill button built on components::button.
inline void pillButton(eui::Ui& ui, const std::string& id, float x, float y,
                       float width, float height, const std::string& label,
                       unsigned int icon, bool primary,
                       std::function<void()> onClick, bool enabled = true,
                       bool danger = false) {
    const Palette& p = palette();
    auto builder = components::button(ui, id);
    builder.theme(themeTokens(), primary)
        .position(x, y)
        .size(width, height)
        .text(label)
        .radius(height * 0.30f)
        .fontSize(13.0f)
        .iconSize(13.0f)
        .opacity(enabled ? 1.0f : 0.45f)
        .transition(uiTransition())
        .onClick(std::move(onClick));

    if (icon != 0) {
        builder.icon(icon);
    }
    if (!primary) {
        const eui::Color labelColor = danger ? p.danger : p.text;
        builder
            .colors(alpha(p.surfaceHover, 0.92f), alpha(p.surfaceActive, 0.95f),
                    alpha(p.surfaceActive, 1.0f))
            .textColor(labelColor)
            .iconColor(labelColor)
            .border(1.0f, alpha(danger ? p.danger : p.border, 0.95f))
            .shadow(0.0f, 0.0f, 0.0f, p.background);
    } else {
        builder.textColor(p.white).iconColor(p.white).border(
            1.0f, alpha(p.accentSoft, 0.55f));
    }
    if (!enabled) {
        builder.disabled(true);
    }
    builder.build();
}

// Small caption used above values, e.g. "正在播放".
inline void sectionLabel(eui::Ui& ui, const std::string& id, float x, float y,
                         float width, const std::string& text) {
    ui.text(id)
        .position(x, y)
        .size(width, 18.0f)
        .text(text)
        .fontSize(12.0f)
        .lineHeight(16.0f)
        .fontWeight(600)
        .color(palette().textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

// Rounded status chip with an optional leading icon.
inline void chip(eui::Ui& ui, const std::string& id, float x, float y,
                 float width, const std::string& text, unsigned int icon,
                 const eui::Color& tint) {
    const float height = 24.0f;
    const float iconSlot = icon != 0 ? 18.0f : 0.0f;
    ui.rect(id)
        .position(x, y)
        .size(width, height)
        .radius(height * 0.5f)
        .color(alpha(tint, 0.14f))
        .border(1.0f, alpha(tint, 0.30f))
        .build();
    if (icon != 0) {
        ui.text(id + ".icon")
            .icon(icon)
            .position(x + 11.0f, y)
            .size(14.0f, height)
            .fontSize(11.0f)
            .color(tint)
            .verticalAlign(eui::VerticalAlign::Center)
            .build();
    }
    ui.text(id + ".text")
        .position(x + 11.0f + iconSlot, y)
        .size(width - 20.0f - iconSlot, height)
        .text(text)
        .fontSize(11.5f)
        .fontWeight(600)
        .color(tint)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

}  // namespace app::ui

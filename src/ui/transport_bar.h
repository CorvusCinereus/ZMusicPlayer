// A Simple Music Player
// Copyright (C) 2026 CorvusCinereus
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "../app_state.h"
#include "theme.h"
#include "widgets.h"

namespace app::ui {

// Bottom transport bar: seek bar, transport buttons, loop mode and volume.
inline void composeTransport(eui::Ui& ui, float width, float top,
                             float height) {
    const Palette& p = palette();
    AppState& s = state();
    const float pad = 24.0f;
    const bool compact = width < 860.0f;

    ui.rect("transport.bg")
        .position(0.0f, top)
        .size(width, height)
        .color(p.surfaceRaised)
        .build();
    hairline(ui, "transport.line", 0.0f, top, width, alpha(p.border, 0.85f));

    const Track* track =
        (s.current >= 0 && s.current < static_cast<int>(s.tracks.size()))
            ? &s.tracks[static_cast<std::size_t>(s.current)]
            : nullptr;
    const double total = s.duration > 0.0
                             ? s.duration
                             : (track != nullptr ? track->duration : 0.0);
    const float fraction =
        s.scrubbing
            ? clamp01(s.scrubRatio)
            : (total > 0.0 ? clamp01(static_cast<float>(s.position / total))
                           : 0.0f);
    const bool seekable = total > 0.0 && s.current >= 0;

    // ---- seek row -----------------------------------------------------------
    const float timeWidth = 56.0f;
    const float timeGap = 14.0f;
    const float row1Y = top + 18.0f;
    const float barX = pad + timeWidth + timeGap;
    const float barWidth =
        std::max(60.0f, width - pad * 2.0f - (timeWidth + timeGap) * 2.0f);
    const float barHeight = 28.0f;
    const float barY = row1Y - 4.0f;
    const float knobSize = 12.0f;
    const float travel = std::max(0.0f, barWidth - knobSize);
    const float knobX = fraction * travel;

    ui.text("transport.elapsed")
        .position(pad, row1Y)
        .size(timeWidth, 20.0f)
        .text(formatTime(s.scrubbing ? fraction * total : s.position))
        .fontSize(12.5f)
        .fontWeight(600)
        .color(s.scrubbing ? p.accentSoft : p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    ui.text("transport.total")
        .position(width - pad - timeWidth, row1Y)
        .size(timeWidth, 20.0f)
        .text(total > 0.0 ? formatTime(total) : std::string("--:--"))
        .fontSize(12.5f)
        .color(p.textMuted)
        .horizontalAlign(eui::HorizontalAlign::Right)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    ui.rect("transport.seek.track")
        .position(barX, row1Y + 8.0f)
        .size(barWidth, 4.0f)
        .radius(2.0f)
        .color(alpha(p.surfaceActive, 0.95f))
        .build();

    ui.rect("transport.seek.fill")
        .position(barX, row1Y + 8.0f)
        .size(seekable ? knobX + knobSize * 0.5f : 0.0f, 4.0f)
        .radius(2.0f)
        .gradient(p.accent, p.violet, eui::GradientDirection::Horizontal)
        .opacity(seekable ? 1.0f : 0.0f)
        .build();

    ui.rect("transport.seek.knob")
        .position(barX + knobX - (seekable ? 0.0f : knobSize * 0.5f),
                  row1Y + 4.0f)
        .size(knobSize, knobSize)
        .radius(knobSize * 0.5f)
        .color(p.white)
        .shadow(10.0f, 0.0f, 2.0f, alpha(p.background, 0.55f))
        .scale(s.scrubbing ? 1.25f : 1.0f)
        .opacity(seekable ? 1.0f : 0.0f)
        .transition(uiTransition())
        .animate(eui::AnimProperty::Transform | eui::AnimProperty::Opacity)
        .build();

    {
        const float hitWidth = barWidth;
        components::mouseArea(ui, "transport.seek.hit")
            .position(barX, barY)
            .size(hitWidth, barHeight)
            .acceptedButtons(eui::PointerButton::Left)
            .onTap([hitWidth](const components::MouseEvent& event) {
                commitScrub(static_cast<float>(event.x) / hitWidth);
            })
            .onDragStart([hitWidth](const components::MouseEvent& event) {
                scrubTo(static_cast<float>(event.x) / hitWidth);
            })
            .onDrag([hitWidth](const components::MouseDragEvent& event) {
                scrubTo(static_cast<float>(event.x) / hitWidth);
            })
            .onDragEnd([hitWidth](const components::MouseDragEvent& event) {
                commitScrub(static_cast<float>(event.x) / hitWidth);
            })
            .build();
    }

    // ---- transport buttons --------------------------------------------------
    const float row2Y = top + 54.0f;
    const float centerX = width * 0.5f;

    const float playSize = 52.0f;
    const float playX = centerX - playSize * 0.5f;
    const float playY = row2Y - (playSize - 48.0f) * 0.5f;

    IconButtonStyle stepStyle;
    stepStyle.size = 40.0f;
    stepStyle.radius = 20.0f;
    stepStyle.iconSize = 15.0f;
    stepStyle.background = alpha(p.surfaceHover, 0.55f);
    stepStyle.hover = alpha(p.surfaceHover, 1.0f);
    stepStyle.pressed = alpha(p.surfaceActive, 1.0f);
    stepStyle.tint = p.text;
    stepStyle.bordered = true;
    iconButton(ui, "transport.previous", 0xF048, playX - 56.0f, row2Y + 4.0f,
               stepStyle, [] { previousTrack(); });
    iconButton(ui, "transport.next", 0xF051, playX + playSize + 16.0f,
               row2Y + 4.0f, stepStyle, [] { nextTrack(); });

    {
        auto play = components::button(ui, "transport.play");
        play.theme(themeTokens(), true)
            .position(playX, playY)
            .size(playSize, playSize)
            .radius(playSize * 0.5f)
            .text("")
            .icon(s.playing ? 0xF04C : 0xF04B)
            .iconSize(20.0f)
            .colors(p.accent, p.accentSoft, p.accentStrong)
            .textColor(p.white)
            .iconColor(p.white)
            .border(1.0f, alpha(p.accentSoft, 0.5f))
            .shadow(20.0f, 0.0f, 7.0f, alpha(p.accent, 0.35f))
            .transition(uiTransition())
            .onClick([] { togglePlay(); })
            .build();
    }

    // Loop mode.
    const std::vector<std::string> loopItems =
        compact ? std::vector<std::string>{"列表", "单曲"}
                : std::vector<std::string>{"列表循环", "单曲循环"};
    const float loopWidth = compact ? 116.0f : 196.0f;
    ui.stack("transport.loop.wrap")
        .position(pad, row2Y + 7.0f)
        .size(loopWidth, 34.0f)
        .content([&] {
            components::segmented(ui, "transport.loop")
                .theme(themeTokens())
                .size(loopWidth, 34.0f)
                .items(loopItems)
                .selected(static_cast<int>(s.loopMode))
                .fontSize(12.5f)
                .transition(uiTransition())
                .onChange([](int value) { setLoopMode(value); })
                .build();
        });

    // Volume cluster.
    const float percentWidth = compact ? 0.0f : 42.0f;
    const float sliderWidth = compact ? 84.0f : 128.0f;
    const float volumeRight = width - pad;
    const float percentX = volumeRight - percentWidth;
    const float sliderX = volumeRight - percentWidth -
                          (percentWidth > 0.0f ? 10.0f : 0.0f) - sliderWidth;
    const float iconX = sliderX - 10.0f - 34.0f;

    unsigned int volumeIcon = 0xF028;
    const float level = s.muted ? 0.0f : s.volume;
    if (level <= 0.001f) {
        volumeIcon = 0xF026;
    } else if (level < 0.5f) {
        volumeIcon = 0xF027;
    }

    IconButtonStyle muteStyle;
    muteStyle.size = 34.0f;
    muteStyle.radius = 10.0f;
    muteStyle.iconSize = 15.0f;
    muteStyle.tint = s.muted ? p.textFaint : p.textMuted;
    muteStyle.hover = alpha(p.surfaceHover, 0.9f);
    muteStyle.pressed = alpha(p.surfaceActive, 0.95f);
    iconButton(ui, "transport.mute", volumeIcon, iconX, row2Y + 7.0f, muteStyle,
               [] { toggleMute(); });

    ui.stack("transport.volume.wrap")
        .position(sliderX, row2Y + 7.0f)
        .size(sliderWidth, 34.0f)
        .content([&] {
            components::slider(ui, "transport.volume")
                .theme(themeTokens())
                .size(sliderWidth, 34.0f)
                .value(level)
                .transition(uiTransition())
                .onChange([](float value) { setVolume(value); })
                .build();
        });

    if (percentWidth > 0.0f) {
        ui.text("transport.percent")
            .position(percentX, row2Y + 7.0f)
            .size(percentWidth, 34.0f)
            .text(
                std::to_string(static_cast<int>(std::lround(level * 100.0f))) +
                "%")
            .fontSize(12.0f)
            .color(p.textMuted)
            .horizontalAlign(eui::HorizontalAlign::Right)
            .verticalAlign(eui::VerticalAlign::Center)
            .build();
    }

    // Shortcut hint.
    const float hintY = top + height - 24.0f;
    const std::string hint =
        "空格 播放/暂停 · ←/→ 快退快进 · ↑/↓ 音量 · Ctrl+←/→ 上一首/下一首 · "
        "Ctrl+O 添加文件 · Ctrl+Shift+O 添加文件夹 · L 循环模式 · M 静音";
    ui.text("transport.hint")
        .position(pad, hintY)
        .size(std::max(0.0f, width - pad * 2.0f), 16.0f)
        .text(elideToWidth(hint, width - pad * 2.0f, 11.5f))
        .fontSize(11.5f)
        .lineHeight(14.0f)
        .color(p.textFaint)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

}  // namespace app::ui

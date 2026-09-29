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
#include <string>

#include "../app_state.h"
#include "theme.h"
#include "widgets.h"

namespace app::ui {

// Album-art style card for the current track.
inline void composeNowPlaying(eui::Ui& ui, float x, float y, float width,
                              float height) {
    const Palette& p = palette();
    const AppState& s = state();
    const float inset = 20.0f;

    ui.rect("now.card")
        .position(x, y)
        .size(width, height)
        .color(p.surface)
        .radius(16.0f)
        .border(1.0f, alpha(p.border, 0.8f))
        .shadow(28.0f, 0.0f, 10.0f, eui::Color(0.0f, 0.0f, 0.0f, 0.30f))
        .build();

    ui.text("now.heading")
        .position(x + inset, y + 15.0f)
        .size(200.0f, 22.0f)
        .text("正在播放")
        .fontSize(15.0f)
        .fontWeight(650)
        .color(p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    const bool hasTrack =
        s.current >= 0 && s.current < static_cast<int>(s.tracks.size());
    const Track* track =
        hasTrack ? &s.tracks[static_cast<std::size_t>(s.current)] : nullptr;

    std::string stateText = "未选择";
    unsigned int stateIcon = 0xF001;
    eui::Color stateTint = p.textMuted;
    if (track != nullptr && s.playing) {
        stateText = "播放中";
        stateIcon = 0xF04B;
        stateTint = p.success;
    } else if (track != nullptr) {
        stateText = "已暂停";
        stateIcon = 0xF04C;
        stateTint = p.accentSoft;
    }

    const float chipWidth = 86.0f;
    chip(ui, "now.state", x + width - inset - chipWidth, y + 14.0f, chipWidth,
         stateText, stateIcon, stateTint);

    // Artwork keeps a square aspect and shrinks with the card.
    const float contentTop = y + 52.0f;
    const float contentHeight = std::max(0.0f, height - 72.0f);
    const float textBlock = 104.0f;
    float artSide =
        std::min(width - inset * 2.0f, contentHeight - textBlock - 12.0f);
    const bool showArt = artSide >= 72.0f;
    if (!showArt) {
        artSide = 0.0f;
    }

    const float artX = x + inset;
    const float artY = contentTop;
    if (showArt) {
        ui.rect("now.art")
            .position(artX, artY)
            .size(artSide, artSide)
            .radius(14.0f)
            .gradient(track != nullptr ? alpha(p.accent, 0.85f)
                                       : alpha(p.surfaceRaised, 1.0f),
                      track != nullptr ? alpha(p.violet, 0.72f)
                                       : alpha(p.surfaceHover, 1.0f),
                      eui::GradientDirection::Horizontal)
            .border(1.0f, alpha(p.border, 0.7f))
            .build();
        ui.text("now.art.icon")
            .icon(track != nullptr ? 0xF001 : 0xF51F)
            .position(artX, artY)
            .size(artSide, artSide)
            .fontSize(artSide * (track != nullptr ? 0.24f : 0.16f))
            .color(alpha(p.white, track != nullptr ? 0.92f : 0.35f))
            .horizontalAlign(eui::HorizontalAlign::Center)
            .verticalAlign(eui::VerticalAlign::Center)
            .build();
    }

    const float textY = showArt ? artY + artSide + 18.0f : contentTop + 4.0f;
    const float textWidth = std::max(0.0f, width - inset * 2.0f);

    ui.text("now.title")
        .position(artX, textY)
        .size(textWidth, 24.0f)
        .text(track != nullptr ? elideToWidth(track->title, textWidth, 16.5f)
                               : std::string("未选择歌曲"))
        .fontSize(16.5f)
        .lineHeight(22.0f)
        .fontWeight(650)
        .color(track != nullptr ? p.text : p.textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    ui.text("now.subtitle")
        .position(artX, textY + 26.0f)
        .size(textWidth, 18.0f)
        .text(track != nullptr
                  ? elideToWidth(
                        track->folder.empty() ? track->path : track->folder,
                        textWidth, 12.0f)
                  : std::string("从播放列表中选择一首开始播放"))
        .fontSize(12.0f)
        .lineHeight(16.0f)
        .color(p.textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    if (track != nullptr) {
        std::string format = fileExtensionOf(track->path);
        for (char& c : format) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        chip(ui, "now.format", artX, textY + 56.0f, 64.0f, format, 0xF1C5,
             p.accentSoft);
        chip(ui, "now.length", artX + 72.0f, textY + 56.0f, 86.0f,
             track->duration > 0.0 ? formatTime(track->duration)
                                   : std::string("--:--"),
             0xF017, p.textMuted);
    }
}

}  // namespace app::ui

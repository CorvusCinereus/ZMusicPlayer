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

#include <cstdint>
#include <string>

#include "../app_state.h"
#include "theme.h"
#include "widgets.h"

namespace app::ui {

// One playlist row. `rowId` is a reusable virtualization slot id, so all
// business state is read through the real `index`.
inline void composePlaylistRow(eui::Ui& rowUi, const std::string& rowId,
                               std::int64_t index, float rowWidth,
                               float rowHeight) {
    AppState& s = state();
    if (index < 0 || index >= static_cast<std::int64_t>(s.tracks.size())) {
        return;
    }

    const Track& track = s.tracks[static_cast<std::size_t>(index)];
    const Palette& p = palette();
    const bool selected = static_cast<int>(index) == s.current;
    const bool sounding = selected && s.playing;

    const eui::Color base =
        selected ? alpha(p.accent, 0.15f)
                 : (index % 2 == 0 ? alpha(p.surfaceRaised, 0.5f)
                                   : eui::Color(0.0f, 0.0f, 0.0f, 0.0f));
    const eui::Color hover =
        selected ? alpha(p.accent, 0.21f) : alpha(p.surfaceHover, 0.85f);
    const eui::Color press =
        selected ? alpha(p.accent, 0.28f) : alpha(p.surfaceActive, 0.95f);

    rowUi.rect(rowId + ".bg")
        .size(rowWidth, rowHeight)
        .radius(10.0f)
        .states(base, hover, press)
        .transition(uiTransition())
        .animate(eui::AnimProperty::Color)
        .onClick([index] { playIndex(static_cast<int>(index)); })
        .build();

    // Selection marker stays in the tree and fades with the selection.
    rowUi.rect(rowId + ".marker")
        .position(0.0f, (rowHeight - 30.0f) * 0.5f)
        .size(3.0f, 30.0f)
        .radius(1.5f)
        .color(p.accent)
        .opacity(selected ? 1.0f : 0.0f)
        .transition(uiTransition())
        .animate(eui::AnimProperty::Opacity)
        .build();

    rowUi.text(rowId + ".num")
        .position(14.0f, 0.0f)
        .size(26.0f, rowHeight)
        .text(std::to_string(index + 1))
        .fontSize(12.5f)
        .color(selected ? p.accentSoft : p.textFaint)
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .opacity(sounding ? 0.0f : 1.0f)
        .transition(uiTransition())
        .animate(eui::AnimProperty::Opacity)
        .build();

    rowUi.text(rowId + ".sounding")
        .icon(0xF0C9)
        .position(14.0f, 0.0f)
        .size(26.0f, rowHeight)
        .fontSize(14.0f)
        .color(p.accent)
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .opacity(sounding ? 1.0f : 0.0f)
        .transition(uiTransition())
        .animate(eui::AnimProperty::Opacity)
        .build();

    const float durationWidth = 52.0f;
    const float removeSize = 28.0f;
    const float removeX = rowWidth - 44.0f;
    const float durationX = removeX - 12.0f - durationWidth;
    const float titleX = 48.0f;
    const float titleWidth = std::max(40.0f, durationX - titleX - 12.0f);

    rowUi.text(rowId + ".title")
        .position(titleX, 9.0f)
        .size(titleWidth, 20.0f)
        .text(elideToWidth(track.title, titleWidth, 14.0f))
        .fontSize(14.0f)
        .lineHeight(19.0f)
        .fontWeight(sounding ? 640 : (selected ? 600 : 520))
        .color(sounding ? p.accentSoft : p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    rowUi.text(rowId + ".folder")
        .position(titleX, 31.0f)
        .size(titleWidth, 16.0f)
        .text(elideToWidth(track.folder, titleWidth, 11.5f))
        .fontSize(11.5f)
        .lineHeight(15.0f)
        .color(p.textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    rowUi.text(rowId + ".duration")
        .position(durationX, 0.0f)
        .size(durationWidth, rowHeight)
        .text(track.duration > 0.0 ? formatTime(track.duration)
                                   : std::string("--:--"))
        .fontSize(12.0f)
        .color(p.textFaint)
        .horizontalAlign(eui::HorizontalAlign::Right)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    IconButtonStyle removeStyle;
    removeStyle.size = removeSize;
    removeStyle.radius = 8.0f;
    removeStyle.iconSize = 11.5f;
    removeStyle.tint = p.textFaint;
    removeStyle.hover = alpha(p.danger, 0.18f);
    removeStyle.pressed = alpha(p.danger, 0.3f);
    iconButton(rowUi, rowId + ".remove", 0xF00D, removeX,
               (rowHeight - removeSize) * 0.5f, removeStyle,
               [index] { removeTrack(static_cast<int>(index)); });
}

// Centered empty state shown inside the playlist card.
inline void composeEmptyState(eui::Ui& ui, float x, float y, float width,
                              float height) {
    const Palette& p = palette();
    const float iconSize = std::min(84.0f, std::max(48.0f, height * 0.26f));
    const float buttonHeight = 40.0f;
    const float blockHeight = iconSize + 150.0f + buttonHeight;
    const float top = y + std::max(8.0f, (height - blockHeight) * 0.5f);
    const float centerX = x + width * 0.5f;

    ui.rect("playlist.empty.glow")
        .position(centerX - iconSize * 0.5f, top)
        .size(iconSize, iconSize)
        .radius(iconSize * 0.5f)
        .gradient(alpha(p.accent, 0.22f), alpha(p.violet, 0.16f),
                  eui::GradientDirection::Horizontal)
        .border(1.0f, alpha(p.border, 0.65f))
        .build();

    ui.text("playlist.empty.icon")
        .icon(0xF001)
        .position(centerX - iconSize * 0.5f, top)
        .size(iconSize, iconSize)
        .fontSize(iconSize * 0.36f)
        .color(alpha(p.accentSoft, 0.95f))
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    ui.text("playlist.empty.title")
        .position(x, top + iconSize + 18.0f)
        .size(width, 24.0f)
        .text("播放列表是空的")
        .fontSize(16.0f)
        .fontWeight(650)
        .color(p.text)
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    ui.text("playlist.empty.desc")
        .position(x, top + iconSize + 46.0f)
        .size(width, 20.0f)
        .text("选择本地音乐文件，或添加整个文件夹自动扫描")
        .fontSize(12.5f)
        .color(p.textMuted)
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    const float buttonWidth = 150.0f;
    const float buttonGap = 12.0f;
    const float buttonsWidth = buttonWidth * 2.0f + buttonGap;
    const float buttonX = centerX - buttonsWidth * 0.5f;
    const float buttonY = top + iconSize + 84.0f;
    pillButton(ui, "playlist.empty.files", buttonX, buttonY, buttonWidth,
               buttonHeight, "选择音乐文件", 0xF15B, true,
               [] { openFilesDialog(); });
    pillButton(ui, "playlist.empty.folder", buttonX + buttonWidth + buttonGap,
               buttonY, buttonWidth, buttonHeight, "添加文件夹", 0xF07C, false,
               [] { openFolderPicker(); });
}

inline void composePlaylistCard(eui::Ui& ui, float x, float y, float width,
                                float height) {
    const Palette& p = palette();
    const AppState& s = state();

    ui.rect("playlist.card")
        .position(x, y)
        .size(width, height)
        .color(p.surface)
        .radius(16.0f)
        .border(1.0f, alpha(p.border, 0.8f))
        .shadow(28.0f, 0.0f, 10.0f, eui::Color(0.0f, 0.0f, 0.0f, 0.30f))
        .build();

    ui.text("playlist.heading")
        .position(x + 20.0f, y + 15.0f)
        .size(220.0f, 22.0f)
        .text("播放列表")
        .fontSize(15.0f)
        .fontWeight(650)
        .color(p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    if (!s.tracks.empty()) {
        ui.text("playlist.count")
            .position(x + width - 20.0f - 160.0f, y + 15.0f)
            .size(160.0f, 22.0f)
            .text("共 " + std::to_string(s.tracks.size()) + " 首")
            .fontSize(12.0f)
            .color(p.textMuted)
            .horizontalAlign(eui::HorizontalAlign::Right)
            .verticalAlign(eui::VerticalAlign::Center)
            .build();
    }

    hairline(ui, "playlist.line", x + 16.0f, y + 50.0f, width - 32.0f,
             alpha(p.border, 0.5f));

    if (s.tracks.empty()) {
        composeEmptyState(ui, x, y + 52.0f, width,
                          std::max(0.0f, height - 58.0f));
        return;
    }

    const float listWidth = std::max(0.0f, width - 20.0f);
    const float listHeight = std::max(0.0f, height - 66.0f);

    components::virtualList(ui, "playlist.list")
        .theme(themeTokens())
        .position(x + 10.0f, y + 56.0f)
        .size(listWidth, listHeight)
        .itemCount(static_cast<std::int64_t>(s.tracks.size()))
        .rowHeight(58.0f)
        .offset(s.playlistScroll)
        .step(116.0f)
        .overscanViewports(0.5f)
        .scrollbarWidth(8.0f)
        .scrollbarGap(6.0f)
        .onChange([](float value) { state().playlistScroll = value; })
        .row([](eui::Ui& rowUi, const std::string& rowId, std::int64_t index,
                float rowWidth, float rowHeight) {
            composePlaylistRow(rowUi, rowId, index, rowWidth, rowHeight);
        })
        .build();
}

}  // namespace app::ui

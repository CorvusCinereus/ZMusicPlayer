#pragma once

#include <string>

#include "../app_state.h"
#include "theme.h"
#include "widgets.h"

namespace app::ui {

// Title, library summary and the three library actions.
inline void composeHeader(eui::Ui& ui, float width, float height) {
    const Palette& p = palette();
    const AppState& s = state();
    const float pad = 24.0f;

    hairline(ui, "header.line", 0.0f, height - 1.0f, width,
             alpha(p.border, 0.55f));

    // Brand mark.
    ui.rect("header.logo")
        .position(pad, 17.0f)
        .size(38.0f, 38.0f)
        .radius(12.0f)
        .gradient(p.accent, p.violet, eui::GradientDirection::Horizontal)
        .shadow(16.0f, 0.0f, 6.0f, alpha(p.accent, 0.28f))
        .build();
    ui.text("header.logo.icon")
        .icon(0xF001)
        .position(pad, 17.0f)
        .size(38.0f, 38.0f)
        .fontSize(17.0f)
        .color(p.white)
        .horizontalAlign(eui::HorizontalAlign::Center)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    // Actions, laid out from the right edge.
    const bool compact = width < 860.0f;
    const float clearWidth = compact ? 40.0f : 92.0f;
    const float folderWidth = compact ? 40.0f : 130.0f;
    const float filesWidth = compact ? 40.0f : 118.0f;
    const float gap = 10.0f;
    const float right = width - pad;
    const float clearX = right - clearWidth;
    const float folderX = clearX - gap - folderWidth;
    const float filesX = folderX - gap - filesWidth;

    const bool hasTracks = !s.tracks.empty();
    pillButton(
        ui, "header.clear", clearX, 17.0f, clearWidth, 38.0f,
        compact ? std::string() : std::string("清空"), 0xF1F8, false,
        [] { state().clearDialogOpen = true; }, hasTracks, true);
    pillButton(ui, "header.folder", folderX, 17.0f, folderWidth, 38.0f,
               compact ? std::string() : std::string("添加文件夹"), 0xF07C,
               false, [] { openFolderPicker(); });
    pillButton(ui, "header.files", filesX, 17.0f, filesWidth, 38.0f,
               compact ? std::string() : std::string("添加文件"), 0xF15B, true,
               [] { openFilesDialog(); });

    // Text block: keep it clear of the action row.
    const float textX = pad + 50.0f;
    const float textWidth = std::max(120.0f, filesX - textX - 16.0f);

    ui.text("header.title")
        .position(textX, 15.0f)
        .size(textWidth, 24.0f)
        .text("音乐播放器")
        .fontSize(19.0f)
        .lineHeight(24.0f)
        .fontWeight(700)
        .color(p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    std::string summary;
    if (s.tracks.empty()) {
        summary = "添加本地音乐文件或整个文件夹，开始你的播放列表";
    } else {
        std::size_t known = 0;
        double total = 0.0;
        for (const Track& track : s.tracks) {
            if (track.duration > 0.0) {
                ++known;
                total += track.duration;
            }
        }
        summary = "共 " + std::to_string(s.tracks.size()) + " 首";
        if (known == s.tracks.size()) {
            summary += " · 总时长 " + formatTime(total);
        } else {
            summary += " · 正在读取时长…";
        }
    }

    ui.text("header.summary")
        .position(textX, 39.0f)
        .size(textWidth, 18.0f)
        .text(elideToWidth(summary, textWidth, 12.0f))
        .fontSize(12.0f)
        .lineHeight(16.0f)
        .color(p.textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

}  // namespace app::ui

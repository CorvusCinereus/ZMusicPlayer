#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "../app_state.h"
#include "theme.h"
#include "widgets.h"

namespace app::ui {

// One subdirectory row of the in-app folder picker.
inline void composeBrowserRow(eui::Ui& rowUi, const std::string& rowId,
                              std::int64_t index, float rowWidth,
                              float rowHeight) {
    const AppState& s = state();
    if (index < 0 || index >= static_cast<std::int64_t>(s.browserDirs.size())) {
        return;
    }
    const Palette& p = palette();
    const std::string directory =
        s.browserDirs[static_cast<std::size_t>(index)];

    rowUi.rect(rowId + ".bg")
        .size(rowWidth, rowHeight)
        .radius(8.0f)
        .states(eui::Color(0.0f, 0.0f, 0.0f, 0.0f), alpha(p.surfaceHover, 0.9f),
                alpha(p.surfaceActive, 0.95f))
        .transition(uiTransition())
        .animate(eui::AnimProperty::Color)
        .onClick([directory] { browserEnter(directory); })
        .build();

    rowUi.text(rowId + ".icon")
        .icon(0xF07C)
        .position(12.0f, 0.0f)
        .size(22.0f, rowHeight)
        .fontSize(13.0f)
        .color(p.accentSoft)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    rowUi.text(rowId + ".name")
        .position(40.0f, 0.0f)
        .size(std::max(40.0f, rowWidth - 56.0f), rowHeight)
        .text(elideToWidth(fileNameOf(directory),
                           std::max(40.0f, rowWidth - 56.0f), 13.5f))
        .fontSize(13.5f)
        .color(p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

// Body of the folder picker dialog; drawn inside the dialog panel, so all
// coordinates are panel-local.
inline void composeFolderPickerBody(eui::Ui& ui, float width, float height) {
    const Palette& p = palette();
    const AppState& s = state();
    const float pad = 22.0f;

    ui.text("folder.title")
        .position(pad, 18.0f)
        .size(std::max(80.0f, width - 120.0f), 26.0f)
        .text("添加音乐文件夹")
        .fontSize(18.0f)
        .fontWeight(650)
        .color(p.text)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    IconButtonStyle closeStyle;
    closeStyle.size = 32.0f;
    closeStyle.radius = 10.0f;
    closeStyle.iconSize = 13.0f;
    closeStyle.tint = p.textMuted;
    closeStyle.hover = alpha(p.surfaceHover, 0.9f);
    closeStyle.pressed = alpha(p.surfaceActive, 0.95f);
    iconButton(ui, "folder.close", 0xF00D, width - pad - 32.0f, 17.0f,
               closeStyle, [] { closeFolderPicker(); });

    // Current directory + navigation.
    const float pathY = 60.0f;
    const float upWidth = 92.0f;
    pillButton(ui, "folder.up", pad, pathY, upWidth, 34.0f, "上一级", 0xF062,
               false, [] { browserGoUp(); });

    const float pathX = pad + upWidth + 10.0f;
    const float pathWidth = std::max(60.0f, width - pad - pathX);
    ui.rect("folder.path.bg")
        .position(pathX, pathY)
        .size(pathWidth, 34.0f)
        .radius(10.0f)
        .color(alpha(p.background, 0.55f))
        .border(1.0f, alpha(p.border, 0.85f))
        .build();
    ui.text("folder.path.text")
        .position(pathX + 12.0f, pathY)
        .size(std::max(20.0f, pathWidth - 24.0f), 34.0f)
        .text(shortenPath(s.browserDir, static_cast<std::size_t>(std::max(
                                            8.0f, (pathWidth - 24.0f) / 7.0f))))
        .fontSize(12.0f)
        .color(p.textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    // Subdirectory list.
    const float listTop = pathY + 46.0f;
    const float footerHeight = 76.0f;
    const float listWidth = std::max(60.0f, width - pad * 2.0f);
    const float listHeight = std::max(80.0f, height - listTop - footerHeight);

    if (s.browserDirs.empty()) {
        ui.text("folder.empty")
            .position(pad, listTop)
            .size(listWidth, listHeight)
            .text("这个文件夹没有子文件夹，可以直接添加它")
            .fontSize(13.0f)
            .color(p.textMuted)
            .horizontalAlign(eui::HorizontalAlign::Center)
            .verticalAlign(eui::VerticalAlign::Center)
            .build();
    } else {
        components::virtualList(ui, "folder.list")
            .theme(themeTokens())
            .position(pad, listTop)
            .size(listWidth, listHeight)
            .itemCount(static_cast<std::int64_t>(s.browserDirs.size()))
            .rowHeight(40.0f)
            .offset(s.browserScroll)
            .step(96.0f)
            .overscanViewports(0.5f)
            .scrollbarWidth(8.0f)
            .scrollbarGap(6.0f)
            .onChange([](float value) { state().browserScroll = value; })
            .row([](eui::Ui& rowUi, const std::string& rowId,
                    std::int64_t index, float rowWidth, float rowHeight) {
                composeBrowserRow(rowUi, rowId, index, rowWidth, rowHeight);
            })
            .build();
    }

    // Footer: what will be added, plus the actions.
    const float footerLine = height - footerHeight;
    hairline(ui, "folder.line", pad, footerLine, listWidth,
             alpha(p.border, 0.6f));

    std::string summary;
    if (s.browserAudioCount > 0) {
        summary =
            "含子文件夹共 " + std::to_string(s.browserAudioCount) + " 首音频";
        if (s.browserTruncated) {
            summary += "（已达扫描上限，可能还有更多）";
        }
    } else if (s.browserTruncated) {
        summary = "扫描已达上限，暂未统计到音频";
    } else {
        summary = "该文件夹中没有找到音频文件";
    }

    const float buttonHeight = 38.0f;
    const float buttonY = height - 54.0f;
    const float addWidth = 148.0f;
    const float cancelWidth = 92.0f;
    const float addX = width - pad - addWidth;
    const float cancelX = addX - 12.0f - cancelWidth;

    ui.text("folder.summary")
        .position(pad, buttonY)
        .size(std::max(60.0f, cancelX - pad - 12.0f), buttonHeight)
        .text(elideToWidth(summary, std::max(60.0f, cancelX - pad - 12.0f),
                           12.0f))
        .fontSize(12.0f)
        .color(p.textMuted)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();

    pillButton(ui, "folder.cancel", cancelX, buttonY, cancelWidth, buttonHeight,
               "取消", 0, false, [] { closeFolderPicker(); });
    pillButton(ui, "folder.add", addX, buttonY, addWidth, buttonHeight,
               "添加此文件夹", 0xF07C, true, [] { browserConfirm(); });
}

// Modal folder picker. EUI-NEO's native file dialog can only select files, so
// directory selection is handled in-app.
inline void composeFolderPicker(eui::Ui& ui, float screenWidth,
                                float screenHeight) {
    const AppState& s = state();
    const float panelWidth =
        std::min(680.0f, std::max(380.0f, screenWidth - 96.0f));
    const float panelHeight =
        std::min(560.0f, std::max(340.0f, screenHeight - 120.0f));

    components::dialog(ui, "folder.picker")
        .theme(themeTokens())
        .screen(screenWidth, screenHeight)
        .size(panelWidth, panelHeight)
        .open(s.folderPickerOpen)
        .transition(panelTransition())
        .onOpenChange([](bool open) {
            if (!open) {
                closeFolderPicker();
            }
        })
        .content([&ui, panelWidth, panelHeight] {
            composeFolderPickerBody(ui, panelWidth, panelHeight);
        })
        .build();
}

}  // namespace app::ui

#pragma once

#include <string>

#include "../app_state.h"
#include "theme.h"
#include "widgets.h"

namespace app::ui {

// Playback/decoder failures.
inline void composeErrorDialog(eui::Ui& ui, float screenWidth,
                               float screenHeight) {
    const AppState& s = state();
    components::dialog(ui, "error.dialog")
        .theme(themeTokens())
        .screen(screenWidth, screenHeight)
        .size(470.0f, 232.0f)
        .open(s.errorDialogOpen)
        .title(s.errorTitle)
        .message(s.errorMessage)
        .primaryText("重新添加")
        .secondaryText("关闭")
        .transition(panelTransition())
        .onPrimary([] {
            dismissError();
            openFilesDialog();
        })
        .onSecondary([] { dismissError(); })
        .onOpenChange([](bool open) {
            if (!open) {
                dismissError();
            }
        })
        .build();
}

// Destructive-action confirmation.
inline void composeClearDialog(eui::Ui& ui, float screenWidth,
                               float screenHeight) {
    const AppState& s = state();
    components::dialog(ui, "clear.dialog")
        .theme(themeTokens())
        .screen(screenWidth, screenHeight)
        .size(440.0f, 214.0f)
        .open(s.clearDialogOpen)
        .title("清空播放列表")
        .message("将从列表中移除全部 " + std::to_string(s.tracks.size()) +
                 " 首歌曲，此操作不可撤销。")
        .primaryText("清空")
        .secondaryText("取消")
        .transition(panelTransition())
        .onPrimary([] { confirmClearPlaylist(); })
        .onSecondary([] { dismissClearDialog(); })
        .onOpenChange([](bool open) {
            if (!open) {
                dismissClearDialog();
            }
        })
        .build();
}

inline void composeToast(eui::Ui& ui, float screenWidth, float screenHeight) {
    const AppState& s = state();
    components::toast(ui, "app.toast")
        .theme(themeTokens())
        .screen(screenWidth, screenHeight)
        .visible(s.toastVisible)
        .size(380.0f, 84.0f)
        .title(s.toastTitle)
        .message(s.toastMessage)
        .icon(s.toastIcon)
        .duration(2.8f)
        .transition(panelTransition())
        .onDismiss([] { dismissToast(); })
        .onAutoDismiss([] { dismissToast(); })
        .build();
}

// Per-frame tick, composed only while something actually needs one: playing,
// scrubbing, or reading the remaining track durations. Idle pages stay
// event-driven so the runtime can sleep.
inline void composeTick(eui::Ui& ui) {
    const AppState& s = state();
    const bool probing = s.probeCursor < s.tracks.size();
    if (!s.playing && !s.scrubbing && !probing) {
        return;
    }
    ui.stack("app.tick")
        .size(1.0f, 1.0f)
        .ignoreLayout()
        .onFrame([](float) { tick(); })
        .build();
}

}  // namespace app::ui

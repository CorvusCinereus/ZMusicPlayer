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

#include <algorithm>

#include "app_state.h"
#include "eui_neo.h"
#include "ui/folder_picker.h"
#include "ui/header_bar.h"
#include "ui/now_playing.h"
#include "ui/overlays.h"
#include "ui/playlist_view.h"
#include "ui/theme.h"
#include "ui/transport_bar.h"

namespace app {

namespace {

constexpr float kHeaderHeight = 72.0f;
constexpr float kTransportHeight = 134.0f;
constexpr float kPadding = 24.0f;
constexpr float kCardGap = 16.0f;

// Window-level shortcuts. UI callbacks already request a re-compose, and so
// does the framework after dispatching these events.
void handleShortcut(const eui::KeyEvent& event) {
    if (!event.isDown()) {
        return;
    }
    const bool shortcut = event.modifiers.control || event.modifiers.super;

    switch (event.key) {
        case eui::InputKey::Space:
            togglePlay();
            break;
        case eui::InputKey::Left:
            if (shortcut) {
                previousTrack();
            } else {
                nudgeSeek(-5.0);
            }
            break;
        case eui::InputKey::Right:
            if (shortcut) {
                nextTrack();
            } else {
                nudgeSeek(5.0);
            }
            break;
        case eui::InputKey::Up:
            nudgeVolume(0.05f);
            break;
        case eui::InputKey::Down:
            nudgeVolume(-0.05f);
            break;
        case eui::InputKey::O:
            if (shortcut) {
                if (event.modifiers.shift) {
                    openFolderPicker();
                } else {
                    openFilesDialog();
                }
            }
            break;
        case eui::InputKey::M:
            toggleMute();
            break;
        case eui::InputKey::L:
            toggleLoopMode();
            break;
        default:
            break;
    }
}

// Playlist and now-playing cards. Wide windows place them side by side, narrow
// ones stack the compact player above the list.
void composeBody(eui::Ui& ui, float width, float height) {
    const float contentWidth = std::max(0.0f, width - kPadding * 2.0f);
    if (contentWidth <= 0.0f || height <= 0.0f) {
        return;
    }

    const bool sideBySide = contentWidth >= 900.0f && height >= 300.0f;
    if (sideBySide) {
        const float nowWidth = std::min(360.0f, contentWidth * 0.34f);
        const float listWidth =
            std::max(0.0f, contentWidth - nowWidth - kCardGap);
        ui::composePlaylistCard(ui, kPadding, 0.0f, listWidth, height);
        ui::composeNowPlaying(ui, kPadding + listWidth + kCardGap, 0.0f,
                              nowWidth, height);
        return;
    }

    const float nowHeight = std::max(130.0f, std::min(210.0f, height * 0.38f));
    const float listHeight = std::max(70.0f, height - nowHeight - kCardGap);
    ui::composeNowPlaying(ui, kPadding, 0.0f, contentWidth, nowHeight);
    ui::composePlaylistCard(ui, kPadding, nowHeight + kCardGap, contentWidth,
                            listHeight);
}

}  // namespace

const DslAppConfig& dslAppConfig() {
    static const DslAppConfig config =
        DslAppConfig{}
            .title("ZMusicPlayer")
            .pageId("music_player")
            .clearColor(eui::Color("#0A0D14"))
            .windowSize(1180, 780)
            .minWindowSize(880, 600)
            .resizable(true)
            .onKeyEvent(
                [](const eui::KeyEvent& event) { handleShortcut(event); });
    return config;
}

void compose(eui::Ui& ui, const eui::Screen& screen) {
    ensureInitialized();

    const float width = screen.width;
    const float height = screen.height;
    const Palette& p = palette();
    const float bodyHeight =
        std::max(0.0f, height - kHeaderHeight - kTransportHeight);

    ui.stack("app.root").size(width, height).content([&] {
        ui.rect("app.background")
            .fill()
            .ignoreLayout()
            .color(p.background)
            .build();

        ui::composeHeader(ui, width, kHeaderHeight);

        if (bodyHeight > 0.0f) {
            ui.stack("app.body")
                .position(0.0f, kHeaderHeight)
                .size(width, bodyHeight)
                .content([&] { composeBody(ui, width, bodyHeight); })
                .build();
        }

        ui.stack("app.transport")
            .position(0.0f, height - kTransportHeight)
            .size(width, kTransportHeight)
            .content([&] {
                ui::composeTransport(ui, width, 0.0f, kTransportHeight);
            })
            .build();
    });

    // Overlays are composed last so they sit above the page.
    ui::composeFolderPicker(ui, width, height);
    ui::composeClearDialog(ui, width, height);
    ui::composeErrorDialog(ui, width, height);
    ui::composeToast(ui, width, height);
    ui::composeTick(ui);
}

}  // namespace app

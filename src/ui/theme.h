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

#include "eui_neo.h"

namespace app {

// Single palette for the whole player: deep navy surfaces, one blue accent and
// a violet/pink pair used for artwork gradients.
struct Palette {
    eui::Color background{"#0A0D14"};
    eui::Color surface{"#131A26"};
    eui::Color surfaceRaised{"#18202E"};
    eui::Color surfaceHover{"#1E2839"};
    eui::Color surfaceActive{"#26324A"};
    eui::Color border{"#28344A"};
    eui::Color text{"#E9EFF8"};
    eui::Color textMuted{"#8E9DB4"};
    eui::Color textFaint{"#5F6D84"};
    eui::Color accent{"#5B8CFF"};
    eui::Color accentStrong{"#4A78E8"};
    eui::Color accentSoft{"#93B4FF"};
    eui::Color violet{"#A46BFF"};
    eui::Color pink{"#FF6FA5"};
    eui::Color danger{"#F2555A"};
    eui::Color success{"#4ED8A0"};
    eui::Color white{"#FFFFFF"};
};

inline const Palette& palette() {
    static const Palette instance;
    return instance;
}

// Replaces the alpha channel, keeping the color.
inline eui::Color alpha(const eui::Color& color, float value) {
    return components::theme::withAlpha(color, value);
}

// Multiplies the existing alpha, keeping the color.
inline eui::Color fade(const eui::Color& color, float value) {
    return components::theme::withOpacity(color, value);
}

inline components::theme::ThemeColorTokens themeTokens() {
    components::theme::ThemeColorTokens tokens = components::theme::dark();
    const Palette& p = palette();
    tokens.background = p.background;
    tokens.primary = p.accent;
    tokens.surface = p.surface;
    tokens.surfaceHover = p.surfaceHover;
    tokens.surfaceActive = p.surfaceActive;
    tokens.text = p.text;
    tokens.border = p.border;
    return tokens;
}

inline eui::Transition uiTransition() {
    return eui::Transition::make(0.18f, eui::Ease::OutCubic);
}

inline eui::Transition panelTransition() {
    return eui::Transition::make(0.26f, eui::Ease::OutCubic);
}

}  // namespace app

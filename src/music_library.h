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

#include <cstddef>
#include <string>
#include <vector>

namespace app {

struct Track {
    std::string path;       // absolute path of the audio file
    std::string title;      // file name without extension
    std::string folder;     // parent directory, shown as the row subtitle
    double duration = 0.0;  // seconds; 0 while still unknown
};

struct ScanOutcome {
    std::vector<std::string> files;
    bool truncated = false;  // a scan cap was reached
};

const std::vector<std::string>& audioExtensions();

bool isAudioFile(const std::string& path);
std::string fileNameOf(const std::string& path);
std::string fileStemOf(const std::string& path);
std::string parentFolderOf(const std::string& path);
std::string fileExtensionOf(const std::string& path);

// "3:07" / "1:02:07"; returns "0:00" for non-positive input.
std::string formatTime(double seconds);

// Shortens a path to fit maxWidth pixels at the given font size, UTF-8 aware.
std::string elideToWidth(const std::string& text, float maxWidth,
                         float fontSize);
// Shortens a long path from the left, keeping the tail: "…/Music/Album".
std::string shortenPath(const std::string& path, std::size_t maxChars);

std::string homeDirectory();
// Absolute, symlink-normalized path used to deduplicate playlist entries.
std::string normalizePath(const std::string& path);
std::string parentDirectory(const std::string& dir);
bool isDirectory(const std::string& path);

Track makeTrack(const std::string& path);

// Recursively collects playable audio files under root, sorted by path.
ScanOutcome scanAudioFiles(const std::string& root, std::size_t maxFiles,
                           std::size_t maxVisited);
// Immediate subdirectories of dir, sorted by name (hidden entries skipped).
std::vector<std::string> listSubdirectories(const std::string& dir);

}  // namespace app

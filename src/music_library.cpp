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

#include "music_library.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>

namespace app {

namespace fs = std::filesystem;

namespace {

std::string toLower(std::string value) {
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool compareNoCase(const std::string& a, const std::string& b) {
    return toLower(a) < toLower(b);
}

bool startsWithDot(const std::string& name) {
    return !name.empty() && name.front() == '.';
}

// Number of UTF-8 code points in the leading `bytes` bytes of `text`.
std::size_t utf8Length(const std::string& text) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < text.size();) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        std::size_t step = 1;
        if ((lead & 0xE0u) == 0xC0u) {
            step = 2;
        } else if ((lead & 0xF0u) == 0xE0u) {
            step = 3;
        } else if ((lead & 0xF8u) == 0xF0u) {
            step = 4;
        }
        i += std::min(step, text.size() - i);
        ++count;
    }
    return count;
}

}  // namespace

const std::vector<std::string>& audioExtensions() {
    static const std::vector<std::string> extensions = {"mp3", "wav", "flac",
                                                        "ogg", "oga"};
    return extensions;
}

std::string fileExtensionOf(const std::string& path) {
    std::string extension = fs::path(path).extension().string();
    if (!extension.empty() && extension.front() == '.') {
        extension.erase(extension.begin());
    }
    return toLower(extension);
}

bool isAudioFile(const std::string& path) {
    const std::string extension = fileExtensionOf(path);
    if (extension.empty()) {
        return false;
    }
    const std::vector<std::string>& allowed = audioExtensions();
    return std::find(allowed.begin(), allowed.end(), extension) !=
           allowed.end();
}

std::string fileNameOf(const std::string& path) {
    return fs::path(path).filename().string();
}

std::string fileStemOf(const std::string& path) {
    return fs::path(path).stem().string();
}

std::string parentFolderOf(const std::string& path) {
    const fs::path parent = fs::path(path).parent_path();
    return parent.empty() ? std::string{} : parent.filename().string();
}

std::string formatTime(double seconds) {
    if (!(seconds > 0.0)) {
        return "0:00";
    }
    const long total = static_cast<long>(seconds + 0.5);
    const long hours = total / 3600;
    const long minutes = (total % 3600) / 60;
    const long secs = total % 60;

    std::string out;
    if (hours > 0) {
        out += std::to_string(hours);
        out += ':';
        if (minutes < 10) {
            out += '0';
        }
    }
    out += std::to_string(minutes);
    out += ':';
    if (secs < 10) {
        out += '0';
    }
    out += std::to_string(secs);
    return out;
}

std::string elideToWidth(const std::string& text, float maxWidth,
                         float fontSize) {
    if (maxWidth <= 0.0f || fontSize <= 0.0f) {
        return {};
    }
    // Rough advance widths: CJK glyphs are full width, latin about half.
    float used = 0.0f;
    std::size_t i = 0;
    std::size_t kept = 0;
    const float ellipsisWidth = fontSize * 0.9f;
    while (i < text.size()) {
        const unsigned char lead = static_cast<unsigned char>(text[i]);
        std::size_t step = 1;
        float advance = fontSize * 0.55f;
        if ((lead & 0xE0u) == 0xC0u) {
            step = 2;
        } else if ((lead & 0xF0u) == 0xE0u) {
            step = 3;
            advance = fontSize;
        } else if ((lead & 0xF8u) == 0xF0u) {
            step = 4;
            advance = fontSize;
        }
        step = std::min(step, text.size() - i);
        if (used + advance + ellipsisWidth > maxWidth) {
            break;
        }
        used += advance;
        i += step;
        kept = i;
    }
    if (i >= text.size()) {
        return text;
    }
    return text.substr(0, kept) + "…";
}

std::string shortenPath(const std::string& path, std::size_t maxChars) {
    if (maxChars == 0) {
        return {};
    }
    const std::size_t length = utf8Length(path);
    if (length <= maxChars || maxChars < 2) {
        return path;
    }
    // Keep the tail, which carries the meaningful directory names.
    std::size_t skip = length - (maxChars - 1);
    std::size_t i = 0;
    while (i < path.size() && skip > 0) {
        const unsigned char lead = static_cast<unsigned char>(path[i]);
        std::size_t step = 1;
        if ((lead & 0xE0u) == 0xC0u) {
            step = 2;
        } else if ((lead & 0xF0u) == 0xE0u) {
            step = 3;
        } else if ((lead & 0xF8u) == 0xF0u) {
            step = 4;
        }
        i += std::min(step, path.size() - i);
        --skip;
    }
    return "…" + path.substr(i);
}

std::string homeDirectory() {
    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        return std::string(home);
    }
    std::error_code error;
    const fs::path current = fs::current_path(error);
    return error ? std::string(".") : current.string();
}

std::string normalizePath(const std::string& path) {
    std::error_code error;
    fs::path normalized = fs::weakly_canonical(fs::path(path), error);
    if (error || normalized.empty()) {
        normalized = fs::absolute(fs::path(path), error);
        if (error || normalized.empty()) {
            return path;
        }
    }
    return normalized.string();
}

std::string parentDirectory(const std::string& dir) {
    const fs::path parent = fs::path(dir).parent_path();
    return parent.empty() ? dir : parent.string();
}

bool isDirectory(const std::string& path) {
    std::error_code error;
    return fs::is_directory(fs::path(path), error);
}

Track makeTrack(const std::string& path) {
    Track track;
    track.path = path;
    track.title = fileStemOf(path);
    track.folder = parentFolderOf(path);
    if (track.title.empty()) {
        track.title = fileNameOf(path);
    }
    return track;
}

ScanOutcome scanAudioFiles(const std::string& root, std::size_t maxFiles,
                           std::size_t maxVisited) {
    ScanOutcome outcome;
    std::error_code error;
    if (!fs::is_directory(fs::path(root), error)) {
        return outcome;
    }

    fs::recursive_directory_iterator iterator(
        fs::path(root), fs::directory_options::skip_permission_denied, error);
    if (error) {
        return outcome;
    }

    const fs::recursive_directory_iterator end;
    std::size_t visited = 0;
    while (iterator != end) {
        if (++visited > maxVisited || outcome.files.size() >= maxFiles) {
            outcome.truncated = true;
            break;
        }
        const fs::directory_entry& entry = *iterator;
        std::error_code entryError;
        if (entry.is_regular_file(entryError)) {
            const std::string path = entry.path().string();
            if (isAudioFile(path)) {
                outcome.files.push_back(path);
            }
        }
        iterator.increment(error);
        if (error) {
            error.clear();
            break;
        }
    }

    std::sort(outcome.files.begin(), outcome.files.end(), compareNoCase);
    return outcome;
}

std::vector<std::string> listSubdirectories(const std::string& dir) {
    std::vector<std::string> directories;
    std::error_code error;
    fs::directory_iterator iterator(
        fs::path(dir), fs::directory_options::skip_permission_denied, error);
    if (error) {
        return directories;
    }
    const fs::directory_iterator end;
    while (iterator != end) {
        const fs::directory_entry& entry = *iterator;
        std::error_code entryError;
        if (entry.is_directory(entryError)) {
            std::string name = entry.path().filename().string();
            if (!startsWithDot(name)) {
                directories.push_back(entry.path().string());
            }
        }
        iterator.increment(error);
        if (error) {
            break;
        }
    }
    std::sort(directories.begin(), directories.end(), compareNoCase);
    return directories;
}

}  // namespace app

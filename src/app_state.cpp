#include "app_state.h"

#include <algorithm>
#include <cstdlib>

#include "eui/platform.h"

namespace app {

namespace {

// Caps keep a runaway directory tree from freezing the UI thread.
constexpr std::size_t kAddFileLimit = 5000;
constexpr std::size_t kAddVisitLimit = 200000;
constexpr std::size_t kCountFileLimit = 999;
constexpr std::size_t kCountVisitLimit = 20000;

float clamp01(float value) { return std::max(0.0f, std::min(value, 1.0f)); }

bool envFlag(const char* name) {
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') {
        return false;
    }
    return std::string(value) != "0";
}

// Splits a colon separated list of files and folders.
std::vector<std::string> splitPathList(const std::string& value) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : value) {
        if (c == ':') {
            if (!current.empty()) {
                parts.push_back(current);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

void refreshBrowser() {
    AppState& s = state();
    s.browserDirs = listSubdirectories(s.browserDir);
    const ScanOutcome outcome =
        scanAudioFiles(s.browserDir, kCountFileLimit, kCountVisitLimit);
    s.browserAudioCount = static_cast<int>(outcome.files.size());
    s.browserTruncated = outcome.truncated;
    s.browserScroll = 0.0f;
}

}  // namespace

AppState& state() {
    static AppState instance;
    return instance;
}

AudioPlayer& player() {
    static AudioPlayer instance;
    return instance;
}

void ensureInitialized() {
    AppState& s = state();
    if (s.initialized) {
        return;
    }
    s.initialized = true;

    const char* open = std::getenv("MUSICPLAYER_OPEN");
    if (open != nullptr && open[0] != '\0') {
        addPaths(splitPathList(open), true);
        if (envFlag("MUSICPLAYER_AUTOPLAY") && !s.tracks.empty()) {
            playIndex(0);
        }
    }
}

void addPaths(const std::vector<std::string>& paths, bool recursive) {
    AppState& s = state();

    std::vector<std::string> collected;
    bool truncated = false;
    for (const std::string& entry : paths) {
        if (entry.empty()) {
            continue;
        }
        if (isDirectory(entry)) {
            if (!recursive) {
                continue;
            }
            const ScanOutcome outcome =
                scanAudioFiles(entry, kAddFileLimit, kAddVisitLimit);
            truncated = truncated || outcome.truncated;
            collected.insert(collected.end(), outcome.files.begin(),
                             outcome.files.end());
        } else if (isAudioFile(entry)) {
            collected.push_back(entry);
        }
    }

    if (collected.empty()) {
        showToast("没有找到音频文件", "支持 MP3、WAV、FLAC 与 OGG 格式。",
                  0xF071);
        return;
    }

    std::vector<std::string> known;
    known.reserve(s.tracks.size() + collected.size());
    for (const Track& track : s.tracks) {
        known.push_back(normalizePath(track.path));
    }

    int added = 0;
    for (const std::string& path : collected) {
        const std::string normalized = normalizePath(path);
        if (std::find(known.begin(), known.end(), normalized) != known.end()) {
            continue;
        }
        known.push_back(normalized);
        s.tracks.push_back(makeTrack(path));
        ++added;
    }

    if (added == 0) {
        showToast("没有新增歌曲", "选中的歌曲已经在播放列表中。", 0xF05A);
        return;
    }

    if (s.current < 0) {
        playIndex(0);
    }
    showToast("已添加到播放列表",
              std::to_string(added) + " 首歌曲" +
                  (truncated ? "（已达扫描上限）" : ""),
              0xF001);
}

void openFilesDialog() {
    const eui::platform::FileDialogResult result =
        eui::platform::openFileDialog(
            {"选择音乐文件", audioExtensions(), {}, "音频文件", true});
    if (result.selected()) {
        addPaths(result.paths, false);
        return;
    }
    if (result.status == eui::platform::FileDialogStatus::Failed) {
        showError("打开文件失败", result.error);
    }
}

void openFolderPicker() {
    AppState& s = state();
    if (s.browserDir.empty()) {
        s.browserDir = homeDirectory();
    }
    refreshBrowser();
    s.folderPickerOpen = true;
}

void closeFolderPicker() { state().folderPickerOpen = false; }

void browserEnter(const std::string& directory) {
    AppState& s = state();
    if (directory.empty()) {
        return;
    }
    s.browserDir = directory;
    refreshBrowser();
}

void browserGoUp() {
    AppState& s = state();
    const std::string parent = parentDirectory(s.browserDir);
    if (parent == s.browserDir) {
        return;
    }
    s.browserDir = parent;
    refreshBrowser();
}

void browserConfirm() {
    AppState& s = state();
    const std::string directory = s.browserDir;
    s.folderPickerOpen = false;
    if (!directory.empty()) {
        addPaths({directory}, true);
    }
}

void clearPlaylist() {
    AppState& s = state();
    player().unload();
    s.tracks.clear();
    s.current = -1;
    s.playing = false;
    s.position = 0.0;
    s.duration = 0.0;
    s.scrubbing = false;
    s.scrubRatio = 0.0f;
    s.playlistScroll = 0.0f;
    s.probeCursor = 0;
    s.clearDialogOpen = false;
    showToast("播放列表已清空", "可以重新添加本地音乐。", 0xF1F8);
}

void playIndex(int index) {
    AppState& s = state();
    if (index < 0 || index >= static_cast<int>(s.tracks.size())) {
        return;
    }

    AudioPlayer& p = player();
    const std::string path = s.tracks[static_cast<std::size_t>(index)].path;
    if (!p.load(path)) {
        showError("无法播放该歌曲", p.error());
        return;
    }
    p.setVolume(s.muted ? 0.0f : s.volume);
    p.setLooping(s.loopMode == LoopMode::SingleRepeat);
    if (!p.play()) {
        showError("无法播放该歌曲", p.error());
        return;
    }

    s.current = index;
    s.playing = true;
    s.scrubbing = false;
    s.position = 0.0;

    const double loaded = p.duration();
    if (loaded > 0.0) {
        s.duration = loaded;
        s.tracks[static_cast<std::size_t>(index)].duration = loaded;
    } else {
        s.duration = s.tracks[static_cast<std::size_t>(index)].duration;
    }
}

void togglePlay() {
    AppState& s = state();
    if (s.current < 0) {
        if (s.tracks.empty()) {
            openFilesDialog();
            return;
        }
        playIndex(0);
        return;
    }

    AudioPlayer& p = player();
    if (s.playing) {
        p.pause();
        s.playing = false;
        return;
    }

    // A finished or unloaded track restarts from the beginning.
    if (!p.loaded() || p.atEnd()) {
        playIndex(s.current);
        return;
    }
    if (!p.play()) {
        showError("无法继续播放", p.error());
        return;
    }
    s.playing = true;
}

void nextTrack() {
    AppState& s = state();
    if (s.tracks.empty()) {
        return;
    }
    const int count = static_cast<int>(s.tracks.size());
    const int next = s.current < 0 ? 0 : (s.current + 1) % count;
    playIndex(next);
}

void previousTrack() {
    AppState& s = state();
    if (s.tracks.empty()) {
        return;
    }
    // Restart the current song first, like a traditional transport.
    if (s.position > 3.0 && player().loaded()) {
        player().seek(0.0);
        s.position = 0.0;
        return;
    }
    const int count = static_cast<int>(s.tracks.size());
    const int previous = s.current <= 0 ? count - 1 : s.current - 1;
    playIndex(previous);
}

void scrubTo(float ratio) {
    AppState& s = state();
    s.scrubbing = true;
    s.scrubRatio = clamp01(ratio);
}

void commitScrub(float ratio) {
    AppState& s = state();
    s.scrubRatio = clamp01(ratio);
    const double total =
        s.duration > 0.0
            ? s.duration
            : (s.current >= 0
                   ? s.tracks[static_cast<std::size_t>(s.current)].duration
                   : 0.0);
    if (total > 0.0 && player().loaded()) {
        const double target = s.scrubRatio * total;
        if (player().seek(target)) {
            s.position = target;
        }
    }
    s.scrubbing = false;
}

void nudgeSeek(double seconds) {
    AppState& s = state();
    if (!player().loaded()) {
        return;
    }
    const double current = s.scrubbing ? s.scrubRatio * s.duration : s.position;
    const double total = s.duration > 0.0 ? s.duration : player().duration();
    const double target = std::max(0.0, std::min(current + seconds, total));
    if (player().seek(target)) {
        s.position = target;
        s.scrubbing = false;
    }
}

void setVolume(float volume) {
    AppState& s = state();
    s.volume = clamp01(volume);
    if (s.volume > 0.0f) {
        s.muted = false;
    }
    player().setVolume(s.muted ? 0.0f : s.volume);
}

void nudgeVolume(float delta) {
    AppState& s = state();
    const float base = s.muted ? 0.0f : s.volume;
    setVolume(base + delta);
}

void toggleMute() {
    AppState& s = state();
    if (s.muted) {
        s.muted = false;
        player().setVolume(s.volume);
        return;
    }
    s.muted = true;
    player().setVolume(0.0f);
}

void setLoopMode(int mode) {
    AppState& s = state();
    const LoopMode requested = mode == static_cast<int>(LoopMode::SingleRepeat)
                                   ? LoopMode::SingleRepeat
                                   : LoopMode::ListLoop;
    if (s.loopMode == requested) {
        return;
    }
    s.loopMode = requested;
    player().setLooping(s.loopMode == LoopMode::SingleRepeat);
}

void toggleLoopMode() {
    AppState& s = state();
    s.loopMode = s.loopMode == LoopMode::ListLoop ? LoopMode::SingleRepeat
                                                  : LoopMode::ListLoop;
    player().setLooping(s.loopMode == LoopMode::SingleRepeat);
}

void removeTrack(int index) {
    AppState& s = state();
    if (index < 0 || index >= static_cast<int>(s.tracks.size())) {
        return;
    }

    const bool removingCurrent = index == s.current;
    const bool wasPlaying = s.playing;

    s.tracks.erase(s.tracks.begin() + index);
    if (s.probeCursor > static_cast<std::size_t>(index)) {
        --s.probeCursor;
    }

    if (s.tracks.empty()) {
        player().unload();
        s.current = -1;
        s.playing = false;
        s.position = 0.0;
        s.duration = 0.0;
        return;
    }

    if (removingCurrent) {
        const int next = std::min(index, static_cast<int>(s.tracks.size()) - 1);
        s.current = -1;
        s.playing = false;
        player().unload();
        s.position = 0.0;
        s.duration = 0.0;
        if (wasPlaying) {
            playIndex(next);
        } else {
            s.current = next;
        }
        return;
    }

    if (index < s.current) {
        --s.current;
    }
}

void showError(const std::string& title, const std::string& message) {
    AppState& s = state();
    s.errorTitle = title;
    s.errorMessage = message;
    s.errorDialogOpen = true;
}

void dismissError() { state().errorDialogOpen = false; }

void dismissClearDialog() { state().clearDialogOpen = false; }

void confirmClearPlaylist() {
    state().clearDialogOpen = false;
    clearPlaylist();
}

void showToast(const std::string& title, const std::string& message,
               unsigned int icon) {
    AppState& s = state();
    s.toastTitle = title;
    s.toastMessage = message;
    s.toastIcon = icon;
    s.toastVisible = true;
}

void dismissToast() { state().toastVisible = false; }

void tick() {
    AppState& s = state();

    // Probe pending durations a few at a time so the playlist gains time labels
    // without blocking a frame.
    if (s.probeCursor < s.tracks.size()) {
        const std::size_t batchEnd =
            std::min(s.tracks.size(), s.probeCursor + 6);
        for (; s.probeCursor < batchEnd; ++s.probeCursor) {
            Track& track = s.tracks[s.probeCursor];
            if (track.duration <= 0.0) {
                track.duration = AudioPlayer::probeDuration(track.path);
            }
        }
    }

    if (!s.playing) {
        return;
    }
    AudioPlayer& p = player();
    if (!p.loaded()) {
        return;
    }

    if (!s.scrubbing) {
        s.position = p.position();
    }
    const double loaded = p.duration();
    if (loaded > 0.0) {
        s.duration = loaded;
        if (s.current >= 0 && s.current < static_cast<int>(s.tracks.size())) {
            s.tracks[static_cast<std::size_t>(s.current)].duration = loaded;
        }
    }

    // Single repeat is handled by the looping sound itself; the list advances
    // when a track reaches its end.
    if (s.loopMode == LoopMode::ListLoop && p.atEnd()) {
        nextTrack();
    }
}

}  // namespace app

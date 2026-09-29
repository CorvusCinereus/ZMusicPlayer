#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "audio_player.h"
#include "music_library.h"

namespace app {

// Playback order. ListLoop is the default: the playlist plays through and
// wraps.
enum class LoopMode {
    ListLoop = 0,
    SingleRepeat = 1,
};

struct AppState {
    std::vector<Track> tracks;
    int current = -1;  // index of the loaded/selected track, -1 when none

    bool playing = false;
    LoopMode loopMode = LoopMode::ListLoop;

    float volume = 0.8f;
    bool muted = false;

    double position = 0.0;
    double duration = 0.0;

    bool scrubbing = false;
    float scrubRatio = 0.0f;

    float playlistScroll = 0.0f;

    // In-app folder picker (EUI-NEO's native dialog cannot select directories).
    bool folderPickerOpen = false;
    std::string browserDir;
    std::vector<std::string> browserDirs;
    int browserAudioCount = 0;
    bool browserTruncated = false;
    float browserScroll = 0.0f;

    // Overlays.
    bool errorDialogOpen = false;
    std::string errorTitle;
    std::string errorMessage;
    bool clearDialogOpen = false;
    bool toastVisible = false;
    std::string toastTitle;
    std::string toastMessage;
    unsigned int toastIcon = 0xF058;

    // Background duration probing: 6 tracks per frame.
    std::size_t probeCursor = 0;

    bool initialized = false;
};

AppState& state();
AudioPlayer& player();

// Reads MUSICPLAYER_OPEN / MUSICPLAYER_AUTOPLAY once on the first compose.
void ensureInitialized();

void addPaths(const std::vector<std::string>& paths, bool recursive);
void openFilesDialog();
void openFolderPicker();
void closeFolderPicker();
void browserEnter(const std::string& directory);
void browserGoUp();
void browserConfirm();

void clearPlaylist();
void playIndex(int index);
void togglePlay();
void nextTrack();
void previousTrack();

void scrubTo(float ratio);
void commitScrub(float ratio);
void nudgeSeek(double seconds);

void setVolume(float volume);
void nudgeVolume(float delta);
void toggleMute();
void setLoopMode(int mode);
void toggleLoopMode();
void removeTrack(int index);

void showError(const std::string& title, const std::string& message);
void dismissError();
void dismissClearDialog();
void confirmClearPlaylist();
void showToast(const std::string& title, const std::string& message,
               unsigned int icon);
void dismissToast();

// Advances the playhead, probes pending durations and auto-advances the list.
void tick();

}  // namespace app

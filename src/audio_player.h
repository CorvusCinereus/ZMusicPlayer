#pragma once

#include <memory>
#include <string>

namespace app {

// Streaming audio player used by the music player UI.
//
// EUI-NEO ships a miniaudio-based player (eui::audio::Player) that has no
// volume control and no looping switch, so the app drives miniaudio directly.
// The bundled miniaudio.h is included as *declarations only*: the
// implementation is the one already compiled into the static eui_neo library
// (3rd/EUI-NEO/core/audio/audio.cpp), so this app adds no duplicate miniaudio
// symbols and the framework submodule stays untouched.
class AudioPlayer {
   public:
    AudioPlayer();
    ~AudioPlayer();

    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // Opens a file without starting playback. Returns false and fills error().
    bool load(const std::string& path);
    void unload();

    bool play();
    bool pause();
    bool toggle();
    bool seek(double seconds);

    // Linear gain in [0, 1]. Applied immediately and kept for the next load.
    bool setVolume(float volume);
    float volume() const { return volume_; }

    // Repeats the current file forever instead of stopping at the end.
    bool setLooping(bool looping);
    bool looping() const { return looping_; }

    bool loaded() const;
    bool playing() const;
    // True once the file reached its end (never true while looping).
    bool atEnd() const;
    double position() const;
    double duration() const;

    const std::string& error() const { return error_; }

    // Decodes only enough of the file to report its length, without touching
    // the shared playback engine. Returns 0 when the file cannot be probed.
    static double probeDuration(const std::string& path);

   private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    float volume_ = 0.8f;
    bool looping_ = false;
    std::string error_;
};

}  // namespace app

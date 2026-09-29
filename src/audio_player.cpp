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

#include "audio_player.h"

#include <algorithm>

#include "miniaudio.h"

namespace app {

namespace {

float clampVolume(float value) { return std::max(0.0f, std::min(value, 1.0f)); }

}  // namespace

struct AudioPlayer::Impl {
    ma_engine engine{};
    ma_sound sound{};
    bool engineReady = false;
    bool soundReady = false;

    ~Impl() {
        releaseSound();
        if (engineReady) {
            ma_engine_uninit(&engine);
        }
    }

    void releaseSound() {
        if (soundReady) {
            ma_sound_uninit(&sound);
            soundReady = false;
        }
    }

    bool ensureEngine() {
        if (engineReady) {
            return true;
        }
        // miniaudio falls back to its null backend when no real device is
        // available, so playback still drives a timeline on machines without
        // a sound card.
        if (ma_engine_init(nullptr, &engine) != MA_SUCCESS) {
            return false;
        }
        engineReady = true;
        return true;
    }
};

AudioPlayer::AudioPlayer() : impl_(std::make_unique<Impl>()) {}
AudioPlayer::~AudioPlayer() = default;

bool AudioPlayer::load(const std::string& path) {
    error_.clear();
    impl_->releaseSound();

    if (path.empty()) {
        error_ = "音频文件路径为空。";
        return false;
    }
    if (!impl_->ensureEngine()) {
        error_ = "无法初始化音频输出设备。";
        return false;
    }

    const ma_result result = ma_sound_init_from_file(
        &impl_->engine, path.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr,
        &impl_->sound);
    if (result != MA_SUCCESS) {
        error_ =
            std::string("无法打开音频文件：") + ma_result_description(result);
        return false;
    }

    impl_->soundReady = true;
    // ma_sound_init_* resets the node volume, so re-apply the current gain.
    ma_sound_set_volume(&impl_->sound, volume_);
    ma_sound_set_looping(&impl_->sound, looping_ ? MA_TRUE : MA_FALSE);
    return true;
}

void AudioPlayer::unload() {
    impl_->releaseSound();
    error_.clear();
}

bool AudioPlayer::play() {
    if (!impl_->soundReady) {
        error_ = "尚未载入音频文件。";
        return false;
    }
    // miniaudio restarts from the beginning when the sound already reached its
    // end, so a finished track replays without an explicit seek.
    const ma_result result = ma_sound_start(&impl_->sound);
    if (result != MA_SUCCESS) {
        error_ = std::string("无法开始播放：") + ma_result_description(result);
        return false;
    }
    error_.clear();
    return true;
}

bool AudioPlayer::pause() {
    if (!impl_->soundReady) {
        error_ = "尚未载入音频文件。";
        return false;
    }
    const ma_result result = ma_sound_stop(&impl_->sound);
    if (result != MA_SUCCESS) {
        error_ = std::string("无法暂停播放：") + ma_result_description(result);
        return false;
    }
    error_.clear();
    return true;
}

bool AudioPlayer::toggle() { return playing() ? pause() : play(); }

bool AudioPlayer::seek(double seconds) {
    if (!impl_->soundReady) {
        error_ = "尚未载入音频文件。";
        return false;
    }
    const double total = duration();
    const double target = total > 0.0 ? std::max(0.0, std::min(seconds, total))
                                      : std::max(0.0, seconds);
    const ma_uint32 sampleRate = ma_engine_get_sample_rate(&impl_->engine);
    if (sampleRate == 0) {
        error_ = "音频引擎未就绪。";
        return false;
    }
    const ma_uint64 frame = static_cast<ma_uint64>(target * sampleRate);
    const ma_result result = ma_sound_seek_to_pcm_frame(&impl_->sound, frame);
    if (result != MA_SUCCESS) {
        error_ =
            std::string("无法调整播放进度：") + ma_result_description(result);
        return false;
    }
    error_.clear();
    return true;
}

bool AudioPlayer::setVolume(float volume) {
    volume_ = clampVolume(volume);
    if (impl_->soundReady) {
        ma_sound_set_volume(&impl_->sound, volume_);
    }
    return true;
}

bool AudioPlayer::setLooping(bool looping) {
    looping_ = looping;
    if (impl_->soundReady) {
        ma_sound_set_looping(&impl_->sound, looping ? MA_TRUE : MA_FALSE);
    }
    return true;
}

bool AudioPlayer::loaded() const { return impl_->soundReady; }

bool AudioPlayer::playing() const {
    return impl_->soundReady && ma_sound_is_playing(&impl_->sound) == MA_TRUE;
}

bool AudioPlayer::atEnd() const {
    return impl_->soundReady && ma_sound_at_end(&impl_->sound) == MA_TRUE;
}

double AudioPlayer::position() const {
    if (!impl_->soundReady) {
        return 0.0;
    }
    ma_uint64 frame = 0;
    if (ma_sound_get_cursor_in_pcm_frames(&impl_->sound, &frame) !=
        MA_SUCCESS) {
        return 0.0;
    }
    const ma_uint32 sampleRate = ma_engine_get_sample_rate(&impl_->engine);
    return sampleRate == 0 ? 0.0 : static_cast<double>(frame) / sampleRate;
}

double AudioPlayer::duration() const {
    if (!impl_->soundReady) {
        return 0.0;
    }
    ma_uint64 frame = 0;
    if (ma_sound_get_length_in_pcm_frames(&impl_->sound, &frame) !=
        MA_SUCCESS) {
        return 0.0;
    }
    const ma_uint32 sampleRate = ma_engine_get_sample_rate(&impl_->engine);
    return sampleRate == 0 ? 0.0 : static_cast<double>(frame) / sampleRate;
}

double AudioPlayer::probeDuration(const std::string& path) {
    ma_decoder decoder;
    if (ma_decoder_init_file(path.c_str(), nullptr, &decoder) != MA_SUCCESS) {
        return 0.0;
    }
    double seconds = 0.0;
    ma_uint64 frames = 0;
    if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) == MA_SUCCESS &&
        decoder.outputSampleRate > 0) {
        seconds = static_cast<double>(frames) /
                  static_cast<double>(decoder.outputSampleRate);
    }
    ma_decoder_uninit(&decoder);
    return seconds;
}

}  // namespace app

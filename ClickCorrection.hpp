#pragma once

#include <Geode/Geode.hpp>
#include <algorithm>
#include <cstdint>

// Small helpers shared by the hooks and the UI.
namespace clickcorrection {
    constexpr int64_t MIN_FRAMES = 1;
    constexpr int64_t MAX_FRAMES = 30;

    // Physics steps per second GD runs at by default; used only for the
    // wall-clock safety net that expires stale buffered clicks.
    constexpr double DEFAULT_TPS = 240.0;

    inline bool isEnabled() {
        return geode::Mod::get()->getSettingValue<bool>("enabled");
    }

    inline void setEnabled(bool value) {
        geode::Mod::get()->setSettingValue<bool>("enabled", value);
    }

    inline int maxFrames() {
        auto value = geode::Mod::get()->getSettingValue<int64_t>("max-frames");
        return static_cast<int>(std::clamp(value, MIN_FRAMES, MAX_FRAMES));
    }

    inline void setMaxFrames(int value) {
        auto clamped = std::clamp<int64_t>(value, MIN_FRAMES, MAX_FRAMES);
        geode::Mod::get()->setSettingValue<int64_t>("max-frames", clamped);
    }

    inline bool showPauseButton() {
        return geode::Mod::get()->getSettingValue<bool>("show-pause-button");
    }
}

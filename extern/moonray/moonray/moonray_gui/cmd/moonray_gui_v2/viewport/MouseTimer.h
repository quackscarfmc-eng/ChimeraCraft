// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <chrono>

namespace moonray_gui_v2 {

// Keeps track of time between clicks
struct MouseTimer {
    std::chrono::steady_clock::time_point mStartTime;           // time at which the mouse click began
    bool mStarted {false};                                      // whether the timer has begun
    int mLastClickDuration {0};                                 // gets the duration of the last click (in ms)

    // Returns whether the click was fast (less than 300 ms)
    bool wasQuickClick() const { return mLastClickDuration < 300; }

    bool wasStarted() const { return mStarted; }

    // Start the timer
    void start(){ 
        if (!mStarted) { 
            mStartTime = std::chrono::steady_clock::now();
            mStarted = true;
        } 
    }

    // End the timer
    void end()
    {
        const std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
        mLastClickDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - mStartTime).count();
        mStarted = false;
    }

    int getDurationMs()
    { 
        const std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - mStartTime).count();
    }
};

} // end namespace moonray_gui_v2

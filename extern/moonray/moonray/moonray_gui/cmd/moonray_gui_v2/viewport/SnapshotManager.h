// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../ColorManager.h"

#include <string>
#include <vector>

namespace moonray { namespace rndr { class RenderContext; }}

namespace moonray_gui_v2 {

class Viewport;

class SnapshotManager {

public:
    SnapshotManager(const std::string& snapPath, const Viewport* viewport);
    ~SnapshotManager() {};

    void takeASnapshot(const moonray::rndr::RenderContext* renderContext);
    void createTexture(float* pixels, const int width, const int height);

    void load();
    void loadSnapshotAtIdx(const int idx);
    void loadNewSnapshot();

    void clear();
    void prev();
    void next();

    int getChosenSnapshotTexture() const;
    bool getSnapshotsAreLoaded() const { return mSnapshotsAreLoaded; }
    const std::vector<unsigned int>& getSnapshotTextures() const { return mSnapshotTextures; }
    const std::vector<std::string>& getSnapshotFilenames() const { return mSnapshotFilenames; }

    int getChosenSnapshotIdx() const { return mChosenSnapshotIdx; }
    void setChosenSnapshotIdx(int i) { mChosenSnapshotIdx = i; }

    void deleteCurrentSnapshot();


private:
    bool mValid {true};                             // whether the snapshot path given is valid
    int mSnapIdx {1};                               // index of the next snapshot to save
    std::string mSnapPath;                          // path to the user's snapshot directory
    bool mSnapshotsAreLoaded {false};               // Have the snapshots been loaded from memory?
    std::vector<unsigned int> mSnapshotTextures;    // Snapshot texture handles
    std::vector<std::string> mSnapshotFilenames;    // Snapshot texture filenames
    int mChosenSnapshotIdx {-1};                    // index of the currently selected snapshot
    ColorManager mColorManager;                     // Color manager for applying CRT to snapshots
    const Viewport* mViewport;                      // Pointer to the viewport for color management
};

} // end namespace moonray_gui_v2

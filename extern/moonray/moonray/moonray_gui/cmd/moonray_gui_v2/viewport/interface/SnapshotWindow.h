// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"

#include <string>

namespace moonray_gui_v2 {

class SnapshotManager;

class SnapshotWindow : public Component {
public:
    SnapshotWindow() : Component(/*isOpen*/ false) {}
    ~SnapshotWindow() override {}

    void draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset) override;

    int getWidth() const override { return -1; }  // -1 means auto-size to full width
    int getHeight() const override { return mHeight; }

private:
    // Define the window settings (size, position, style, etc)
    void configureWindow(const ImVec2& dockOffset) const;

    void highlightSnapshot() const;
    void handleSnapshotHover(const std::string& filename) const;
    void handleSnapshotClick(int chosenSnapshotIdx, int clickedSnapshot) const;

    SnapshotManager* mSnapshotManager {nullptr};    // Pointer to the snapshot manager

    int mHeight {200};                              // height of the window; width is auto-sized to the viewport
    int mBottomPadding {50};                        // padding between the bottom of the window and the snapshots

    int mSnapshotHeight {150};                      // height of the snapshot display; width is then calculated
                                                    // based on the aspect ratio

    float mSelectedBorderThickness {3.f};                     // thickness of the border around the current snapshot
    ImU32 mSelectedBorderColor {IM_COL32(0, 255, 255, 255)};  // color of the border around the current snapshot

};

} // end namespace moonray_gui_v2

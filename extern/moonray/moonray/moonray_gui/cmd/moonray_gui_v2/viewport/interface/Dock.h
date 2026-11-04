// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Component.h"

#include <vector>

namespace moonray_gui_v2 {

/* Abstract base class for a "dock", which here refers to an area affixed to the edge of the viewport
   that expands and contracts independent from the image display area. When the Dock expands/contracts, 
   it affects the size of the viewport. For example, a horizontal dock at the bottom of the viewport
   may contain a status bar that shows information about the current render. When the status bar is
   opened/closed, the dock expands/contracts, and the viewport height is adjusted accordingly. 
   
   There may be multiple components in a dock (e.g. status bar, snapshot window, etc), and each component
   may be opened/closed independently. The dock keeps track of the total size of all open components,
   and provides methods to get the offset of each component within the dock. The order that the components 
   are drawn, from the edge of the viewport inward, is the same order that they are added.
   In other words, if a status bar is added first, and then a snapshot window is added second, the 
   status bar will be drawn at the edge of the viewport, and the snapshot window will be drawn 
   above it (i.e. further inward from the edge).

   NOTE: imgui has a "docking" feature that allows windows to be docked to the main viewport or other windows.
   However, while we use the term "docked" here, it does not refer to imgui's docking feature. Instead, it
   simply refers to stationary windows that affect the size of the viewport, and they are managed manually by us.
   When the imgui "docking" feature is further along in development, we may switch to using that instead.
*/
class Dock {
public:
    Dock();

    // Calculate & store the total size of all open components in the dock,
    // along with the diff from the last size calculation
    int updateSize();

    // Get the offset of a given component within the dock
    // (i.e. the distance from whatever edge of the viewport the dock is attached to)
    int getOffset(const Component* component) const;

    // Add a component to the dock
    void addComponent(const Component* component);

    // Get the size of a given component. This is the only pure virtual method,
    // as it depends on whether the dock is horizontal or vertical (i.e. whether
    // we measure width or height).
    virtual int getComponentSize(const Component* component) const = 0;

    // Get the size of the dock (i.e. width or height, depending on orientation)
    int getSize() const { return mSize; }

    // Get the size difference since the last calculation
    // This is useful for determining if the viewport needs to be resized
    int getSizeDiff() const { return mSizeDiff; }

    // Check if the size has changed since the last calculation
    bool hasChanged() const { return mSizeDiff != 0; }

private:
    std::vector<const Component*> mComponents;  // Components in the dock
    int mSize {0};                              // Current size of the dock
    int mSizeDiff {0};                          // Difference in size since last calculation
};

// This derived class represents a horizontal dock (i.e. docked to the top or bottom of the viewport).
// In this case, the size of the dock is determined by the height of its components.
class HorizontalDock : public Dock {
public:
    HorizontalDock() = default;
    ~HorizontalDock() = default;

    int getComponentSize(const Component* component) const override {
        return component->getHeight();
    }

    int getHeight() const { return getSize(); }
    int getHeightDiff() const { return getSizeDiff(); }
    void updateHeight() { updateSize(); }
};

// This derived class represents a vertical dock (i.e. docked to the left or right of the viewport).
// In this case, the size of the dock is determined by the width of its components.
class VerticalDock : public Dock {
public:
    VerticalDock() = default;
    ~VerticalDock() = default;

    int getComponentSize(const Component* component) const override {
        return component->getWidth();
    }

    int getWidth() const { return getSize(); }
    int getWidthDiff() const { return getSizeDiff(); }
    void updateWidth() { updateSize(); }
};

} // end namespace moonray_gui_v2

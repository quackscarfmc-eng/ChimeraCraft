// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "SnapshotWindow.h"

#include "../SnapshotManager.h"
#include "../Viewport.h"

namespace moonray_gui_v2 {

void
SnapshotWindow::highlightSnapshot() const
{
    // Highlight the image with a border
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 rectMin = ImGui::GetItemRectMin();
    ImVec2 rectMax = ImGui::GetItemRectMax();
    drawList->AddRect(rectMin, rectMax, mSelectedBorderColor, 0.0f, 0, mSelectedBorderThickness);
}

void
SnapshotWindow::handleSnapshotHover(const std::string& filename) const
{
    if (ImGui::IsItemHovered()) {
        // Change mouse cursor to hand when hovering over image
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        // Highlight the image with a border
        highlightSnapshot();

        // Show tooltip with filename
        ImGui::BeginTooltip();
        ImGui::Text("%s", filename.c_str());
        ImGui::EndTooltip();
    }
}

void 
SnapshotWindow::handleSnapshotClick(const int chosenSnapshotIdx, const int clickedSnapshot) const
{
    if (ImGui::IsItemClicked()) {
        // Select the current snapshot to display in main viewport
        // if the selected image was already chosen, unselect it
        if (chosenSnapshotIdx == clickedSnapshot) {
            mSnapshotManager->setChosenSnapshotIdx(-1);
        } else {
            mSnapshotManager->setChosenSnapshotIdx(clickedSnapshot);
        }
    }
}

void
SnapshotWindow::configureWindow(const ImVec2& dockOffset) const
{
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    // Position at bottom of viewport, above any other horizontally docked components
    ImVec2 pos = ImVec2(0, imguiViewport->Size.y - dockOffset.y - mHeight);
    ImVec2 size = ImVec2(imguiViewport->Size.x, mHeight);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
}

void
SnapshotWindow::draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset)
{
    // There must be a valid snapshot manager to draw the window
    if (!mSnapshotManager) {
        mSnapshotManager = viewport->getSnapshotManager();
        if (!mSnapshotManager) {
            return;
        }
    }

    if (!mOpen) { return; }

    configureWindow(dockOffset);

    float aspectRatio = static_cast<float>(viewport->getFramebufferWidth()) / 
                        static_cast<float>(viewport->getFramebufferHeight());
    int snapshotWidth = mSnapshotHeight * aspectRatio;   // width for snapshot display

    ImGui::Begin("Snapshot Window", &mOpen, ImGuiWindowFlags_NoMove | 
                                            ImGuiWindowFlags_NoResize |
                                            ImGuiWindowFlags_HorizontalScrollbar |
                                            ImGuiWindowFlags_NoCollapse);
    
    const auto& snapshotTextures = mSnapshotManager->getSnapshotTextures();
    const auto& snapshotFilenames = mSnapshotManager->getSnapshotFilenames();
    const int chosenSnapshotIdx = mSnapshotManager->getChosenSnapshotIdx();
                                            
    // Render the snapshot(s)
    for (int i = 0; i < snapshotTextures.size(); ++i) {
        // Get current drawing position
        ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
        // Add spacing between images
        ImGui::SameLine(cursorScreenPos.x + (10 + snapshotWidth) * i);
        // Display image (upside down because our images are stored upside down -- this is 
        // because the main texture display in ImageWindow expects an upside down image, and if 
        // a snapshot image is selected, we want it to be displayed right-side-up)
        ImGui::Image((ImTextureID)snapshotTextures[i], ImVec2(snapshotWidth, mSnapshotHeight),
                      ImVec2(0, 1), ImVec2(1, 0));

        // if the snapshot is currently chosen,
        // add a border to indicate it
        if (chosenSnapshotIdx == i) {
            highlightSnapshot();
        }

        handleSnapshotHover(snapshotFilenames[i]);
        handleSnapshotClick(chosenSnapshotIdx, i);
    }
    ImGui::SameLine(ImGui::GetCursorScreenPos().x + mBottomPadding); // add padding at end
    ImGui::End();
}

} // namespace moonray_gui_v2

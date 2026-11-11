// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "PathVisualizerWindow.h"

#include "DrawUtil.h"
#include "../Viewport.h"

#include <moonray/rendering/rndr/PathVisualizerManager.h>

namespace moonray_gui_v2 {

void
PathVisualizerWindow::drawEnableButton()
{
    const bool enabled = mManager->isOn();
    const char* buttonText = enabled ? "Disable" : "Enable";
    const ImVec2 buttonSize(60, 30);
    
    if (ImGui::Button(buttonText, buttonSize)) {
        // When clicked, toggle on/off path visualizer
        if (enabled) { mManager->turnOff(); } 
        else         { mManager->turnOn();  }
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
}

void
PathVisualizerWindow::drawPixelSelector()
{
    // Default to open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Pixel Selector")) {
        const int pixelX = static_cast<int>(mManager->getPixelX());
        const int pixelY = static_cast<int>(mManager->getPixelY());

        drawInputInt("Pixel X", pixelX, mMinPixelX, mMaxPixelX, [&](int value) {
            mManager->setPixelX(static_cast<uint32_t>(std::max(0, value)), /*update*/ true);
        });

        drawInputInt("Pixel Y", pixelY, mMinPixelY, mMaxPixelY, [&](int value) {
            mManager->setPixelY(static_cast<uint32_t>(std::max(0, value)), /*update*/ true);
        });
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
}

void
PathVisualizerWindow::drawSamplingSettingsMenu()
{
    // Default to open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Sampling Settings")) {
        bool useSceneSamples = mManager->getUseSceneSamples();

        if (ImGui::Checkbox("Use Scene Sampling Settings", &useSceneSamples)) {
            mManager->setUseSceneSamples(useSceneSamples, /*update*/ true);
        }
        
        // Disable other sampling settings if useSceneSamples is checked
        ImGui::BeginDisabled(useSceneSamples);

        // -------------------- Pixel Samples Input --------------------------- //
        drawInputInt("Pixel Samples", mManager->getPixelSamples(), mMinPixelSamples, mMaxPixelSamples, [&](int value) {
            mManager->setPixelSamples(value, /*update*/ true);
        });
        
        // -------------------- Light Samples Input --------------------------- //
        drawInputInt("Light Samples", mManager->getLightSamples(), mMinLightSamples, mMaxLightSamples, [&](int value) {
            mManager->setLightSamples(value, /*update*/ true);
        });
        
        // -------------------- BSDF Samples Input --------------------------- //
        drawInputInt("BSDF  Samples", mManager->getBsdfSamples(), mMinBsdfSamples, mMaxBsdfSamples, [&](int value) {
            mManager->setBsdfSamples(value, /*update*/ true);
        });

        ImGui::EndDisabled();
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);}
}

void 
PathVisualizerWindow::drawMaxDepthMenu()
{
    // Default to open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Max Depth")) {

        drawInputInt("Max Depth", mManager->getMaxDepth(), mMinMaxDepth, mMaxMaxDepth, [&](int value) {
            mManager->setMaxDepth(value, /*update*/ true);
        });
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
}

void
PathVisualizerWindow::drawVisibilityTogglesMenu()
{
    // Default to open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Visibility Toggles")) {
        bool showDirectRays = mManager->getShowDirectRays();
        bool showIndirectRays = mManager->getShowIndirectRays();
        bool showSamples = mManager->getShowSamples();

        // Indirect rays (continuing rays)
        bool showIndirectDiffuse = mManager->getShowIndirectDiffuseRays();
        bool showIndirectSpecular = mManager->getShowIndirectSpecularRays();
        
        // Direct rays (occlusion rays)
        bool showDirectDiffuse = mManager->getShowDirectDiffuseRays();
        bool showDirectSpecular = mManager->getShowDirectSpecularRays();
        bool showDirectLight = mManager->getShowDirectLightRays();
        
        // Samples
        bool showDiffuseSamples = mManager->getShowDiffuseSamples();
        bool showSpecularSamples = mManager->getShowSpecularSamples();
        bool showLightSamples = mManager->getShowLightSamples();

        // Parent checkbox for Indirect Rays
        if (ImGui::Checkbox("Indirect Rays (Continuing)", &showIndirectRays)) {
            mManager->setShowIndirectRays(showIndirectRays);
        }
        
        // Indent child checkboxes
        ImGui::Indent();
        if (ImGui::Checkbox("Diffuse", &showIndirectDiffuse)) {
            mManager->setShowIndirectDiffuseRays(showIndirectDiffuse);
        }
        if (ImGui::Checkbox("Specular", &showIndirectSpecular)) {
            mManager->setShowIndirectSpecularRays(showIndirectSpecular);
        }
        ImGui::Unindent();

        ImGui::Spacing();
        
        // Parent checkbox for Direct Rays
        if (ImGui::Checkbox("Direct Rays (Occlusion)", &showDirectRays)) {
            mManager->setShowDirectRays(showDirectRays);
        }
        
        // Indent child checkboxes
        ImGui::Indent();
        if (ImGui::Checkbox("Diffuse##Direct", &showDirectDiffuse)) {
            mManager->setShowDirectDiffuseRays(showDirectDiffuse);
        }
        if (ImGui::Checkbox("Specular##Direct", &showDirectSpecular)) {
            mManager->setShowDirectSpecularRays(showDirectSpecular);
        }
        if (ImGui::Checkbox("Light", &showDirectLight)) {
            mManager->setShowDirectLightRays(showDirectLight);
        }
        ImGui::Unindent();

        ImGui::Spacing();
        
        // Parent checkbox for Samples
        if (ImGui::Checkbox("Samples", &showSamples)) {
            mManager->setShowSamples(showSamples);
        }
        
        // Indent child checkboxes
        ImGui::Indent();
        if (ImGui::Checkbox("Diffuse##Samples", &showDiffuseSamples)) {
            mManager->setShowDiffuseSamples(showDiffuseSamples);
        }
        if (ImGui::Checkbox("Specular##Samples", &showSpecularSamples)) {
            mManager->setShowSpecularSamples(showSpecularSamples);
        }
        if (ImGui::Checkbox("Light##Samples", &showLightSamples)) {
            mManager->setShowLightSamples(showLightSamples);
        }
        ImGui::Unindent();
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
}

void
PathVisualizerWindow::drawStyleMenu(Viewport* viewport)
{
    // Default to open
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Style Options")) {
        // Line width control
        int roundedLineWidth = static_cast<uint32_t>(mManager->getLineWidth() + 0.5f);
        drawInputInt("Line Width", roundedLineWidth, mMinLineWidth, mMaxLineWidth, [&](int value) {
            mManager->setLineWidth(static_cast<float>(value));
        });

        // Hidden line opacity
        drawSliderFloat("Hidden Line Opacity", mManager->getHiddenLineOpacity(), 0.0f, 1.0f, [&](float value) {
            mManager->setHiddenLineOpacity(value);
        });

        // Toggle off/on showing endpoints only
        bool showOnlyEndpoints = viewport->getPVShowOnlyEndpoints();
        if (ImGui::Checkbox("Show Endpoints Only", &showOnlyEndpoints)) {
            viewport->setPVShowOnlyEndpoints(showOnlyEndpoints);
        }


        // Separator between display options and color pickers
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Color pickers for different ray types
        drawColorEdit("Camera Ray", mManager->getCameraRayColor(), [&](scene_rdl2::math::Color color) {
            mManager->setCameraRayColor(color);
        });
        
        ImGui::Spacing();
        ImGui::Text("Indirect Rays (Continuing):");
        ImGui::Indent();
        drawColorEdit("Diffuse", mManager->getIndirectDiffuseRayColor(), [&](scene_rdl2::math::Color color) {
            mManager->setIndirectDiffuseRayColor(color);
        });
        drawColorEdit("Specular", mManager->getIndirectSpecularRayColor(), [&](scene_rdl2::math::Color color) {
            mManager->setIndirectSpecularRayColor(color);
        });
        ImGui::Unindent();
        
        ImGui::Spacing();
        ImGui::Text("Direct Rays (Occlusion):");
        ImGui::Indent();
        drawColorEdit("Diffuse##Direct", mManager->getDirectDiffuseRayColor(), [&](scene_rdl2::math::Color color) {
            mManager->setDirectDiffuseRayColor(color);
        });
        drawColorEdit("Specular##Direct", mManager->getDirectSpecularRayColor(), [&](scene_rdl2::math::Color color) {
            mManager->setDirectSpecularRayColor(color);
        });
        drawColorEdit("Light", mManager->getDirectLightRayColor(), [&](scene_rdl2::math::Color color) {
            mManager->setDirectLightRayColor(color);
        });
        ImGui::Unindent();
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
}

void
PathVisualizerWindow::drawMiscMenu()
{
    // Draw miscellaneous menu (including max ray length)
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Miscellaneous")) {
        drawInputFloat("Max Ray Length", mManager->getMaxRayLength(), [&](float value) {
            mManager->setMaxRayLength(value, /*update*/ true);
        });
    }
    if (ImGui::IsItemHovered()) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
}

void
PathVisualizerWindow::drawNodeInfo()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Node Info")) {
        const int nodeIndex = mManager->getSelectedNode();
        if (nodeIndex < 0) {
            const std::string infoStr = "No node selected. In order to select a node, use the LEFT/RIGHT arrow "
                                        "keys while holding SHIFT";
            ImGui::TextWrapped("%s", infoStr.c_str());
            return;
        }

        const std::string nodeInfo = mManager->getNodeInfo(static_cast<size_t>(nodeIndex));
        ImGui::TextWrapped("%s", nodeInfo.c_str());
    }
}

void
PathVisualizerWindow::configureWindow(const ImVec2& dockOffset) const
{
    // Establish size and position
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(imguiViewport->Size.x - mWidth - dockOffset.x, 0));
    ImGui::SetNextWindowSize(ImVec2(mWidth, imguiViewport->Size.y));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(mWindowPadding, mWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, mItemSpacing));
}

void
PathVisualizerWindow::draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset)
{
    if (!mOpen) { return; }

    // If path visualizer manager doesn't exist, we can't
    // perform any of the actions in the UI. Try
    // to retrieve it from the viewport.
    if (!mManager) {
        mManager = viewport->getPathVisualizerManager();
        if (!mManager) { return; }
    }

    configureWindow(dockOffset);

    // Create window
    ImGui::Begin("Path Visualizer", &mOpen, ImGuiWindowFlags_NoMove | 
                                            ImGuiWindowFlags_NoResize |
                                            ImGuiWindowFlags_NoCollapse);
    // Create "on/off" button
    drawEnableButton();
    // Create pixel selector
    drawPixelSelector();
    // Create sampling settings menu
    drawSamplingSettingsMenu();
    // Create max depth menu
    drawMaxDepthMenu();
    // Create visibility toggles menu
    drawVisibilityTogglesMenu();
    // Create style options menu
    drawStyleMenu(viewport);
    // Create miscellaneous menu
    drawMiscMenu();
    // Create node information display
    drawNodeInfo();

    ImGui::End();
    ImGui::PopStyleVar(2);
}

} // namespace moonray_gui_v2

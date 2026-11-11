// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "HelpWindow.h"

#include "../Viewport.h"
#include "../Keyboard.h"

namespace moonray_gui_v2 {

// Colors for the hotkey tables
constexpr ImVec4 HEADER_COLOR = ImVec4(0.8f, 0.8f, 0.5f, 1.0f);
constexpr ImVec4 HOTKEY_COLOR = ImVec4(0.5f, 1.0f, 1.0f, 1.0f);
constexpr ImVec4 UNBOUND_COLOR = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
// Colors for the status bar diagram
constexpr ImVec4 OUTLINE_COLOR = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
constexpr ImVec4 SECTION_COLORS[] = {
    ImVec4(0.3f, 0.7f, 1.0f, 1.0f),
    ImVec4(0.7f, 1.0f, 0.3f, 1.0f),
    ImVec4(1.0f, 0.7f, 0.3f, 1.0f),
    ImVec4(1.0f, 0.5f, 0.8f, 1.0f),
    ImVec4(0.5f, 1.0f, 0.8f, 1.0f),
    ImVec4(1.0f, 1.0f, 0.5f, 1.0f),
    ImVec4(0.8f, 0.8f, 0.8f, 1.0f)
};

void
HelpWindow::configureWindow() const
{
    // Set the help window to the full viewport size
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imguiViewport->Pos);
    ImGui::SetNextWindowSize(imguiViewport->Size);
}

void
HelpWindow::draw(Viewport* viewport, const ImVec2& /*currentPixel*/, const ImVec2& /*dockOffset*/)
{
    if (!mOpen) { return; }

    configureWindow();

    ImGui::Begin("MoonRay GUI Info", &mOpen, ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoResize |
                                             ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoSavedSettings);

    if (ImGui::BeginTabBar("HelpTabs")) {
        // Camera Controls Tab
        if (ImGui::BeginTabItem("Camera")) {
            drawCameraControls(viewport);
            ImGui::EndTabItem();
        }

        // Color Management Tab
        if (ImGui::BeginTabItem("Color Management")) {
            drawColorManagementControls(viewport);
            ImGui::EndTabItem();
        }

        // Command Line Tab
        if (ImGui::BeginTabItem("Command Line")) {
            drawCommandLineOptions();
            ImGui::EndTabItem();
        }

        // Denoising Tab
        if (ImGui::BeginTabItem("Denoising")) {
            drawDenoiseControls(viewport);
            ImGui::EndTabItem();
        }

        // Fast Progressive Tab
        if (ImGui::BeginTabItem("Fast Progressive")) {
            drawFastProgressiveControls(viewport);
            ImGui::EndTabItem();
        }

        // Output Controls Tab
        if (ImGui::BeginTabItem("Output")) {
            drawOutputControls(viewport);
            ImGui::EndTabItem();
        }

        // Path Visualizer Tab
        if (ImGui::BeginTabItem("Path Visualizer")) {
            drawPathVisualizerControls(viewport);
            ImGui::EndTabItem();
        }

        // Status Bar Tab
        if (ImGui::BeginTabItem("Status Bar")) {
            drawStatusBarInfo();
            ImGui::EndTabItem();
        }

        // Viewport Controls Tab
        if (ImGui::BeginTabItem("Viewport")) {
            drawViewportControls(viewport);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}


void drawHotkeyEntry(Viewport* viewport, const Action action)
{
    if (!viewport) { return; }

    const Keyboard* keyboard = viewport->getKeyboard();
    if (!keyboard) { return; }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (action == ACTION_IMAGE2D_ZOOM) {
        // For now, ACTION_IMAGE2D_ZOOM is always bound to mouse scroll,
        // and does not offer any user customizations, so we can just hardcode the display here.
        ImGui::TextColored(HOTKEY_COLOR, "%s", "Scroll");
    } else {
        // Get the key binding for this action
        KeyModPair keyMod = keyboard->getKeyModPair(action);
        
        if (keyMod.first != -1) {
            std::string displayStr = getModifierName(keyMod.second) + getKeyName(keyMod.first);
            ImGui::TextColored(HOTKEY_COLOR, "%s", displayStr.c_str());
        } else {
            ImGui::TextColored(UNBOUND_COLOR, "[UNBOUND]");
        }
    }

    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", getDescription(action).c_str());
}

void drawHotkeyTable(const std::set<Action>& actions, Viewport* viewport, const char* tableID)
{
    if (ImGui::BeginTable(tableID, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Hotkey", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto& action : actions) {
            drawHotkeyEntry(viewport, action);
        }

        ImGui::EndTable();
    }
}

void
HelpWindow::drawCameraControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "MoonRay GUI provides two camera modes: Orbit and Freecam. Use the hotkeys below to control "
                          "the camera and navigate the scene. All hotkeys can be customized in the Key Bindings window, "
                          "and will be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    const std::vector<std::string> cameraCategories = {"General Camera Controls", 
                                                       "Camera Movement Keys", 
                                                       "Camera Mouse Controls"};
    for (const auto& catName : cameraCategories) {
        if (ACTION_CATEGORIES.find(catName) != ACTION_CATEGORIES.end()) {
            ImGui::TextColored(HEADER_COLOR, "%s", catName.c_str());
            drawHotkeyTable(ACTION_CATEGORIES.at(catName), viewport, catName.c_str());
            ImGui::Spacing();
        }
    }
}

void
HelpWindow::drawColorManagementControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "MoonRay GUI uses OpenColorIO for color management. You can set the OCIO config filepath "
                          "using the OCIO environment variable. You can also temporarily adjust the exposure "
                          "and gamma of the displayed image using the hotkeys below. Note that these can be "
                          "customized in the Key Bindings window, and any changes will be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    const std::vector<std::string> colorCategories = {"Exposure Controls", "Gamma Controls"};
    for (const auto& catName : colorCategories) {
        if (ACTION_CATEGORIES.find(catName) != ACTION_CATEGORIES.end()) {
            ImGui::TextColored(HEADER_COLOR, "%s", catName.c_str());
            drawHotkeyTable(ACTION_CATEGORIES.at(catName), viewport, catName.c_str());
            ImGui::Spacing();
        }
    }
}

void
HelpWindow::drawCommandLineOptions() const
{
    ImGui::Spacing();
    ImGui::TextWrapped("Start using moonray_gui with these command line options. Use -h flag for a complete list.");
    ImGui::Spacing();

    auto commandLineTable = [&] (const char* tableID, const std::vector<std::pair<std::string, std::string>>& options) {
        if (ImGui::BeginTable(tableID, 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Option", ImGuiTableColumnFlags_WidthFixed, 200);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& option : options) {
                ImGui::TableNextRow(); 
                ImGui::TableNextColumn(); 
                ImGui::Text("%s", option.first.c_str()); 
                ImGui::TableNextColumn();
                ImGui::TextWrapped("%s", option.second.c_str());
            }
            ImGui::EndTable();
        }
    };

    ImGui::TextColored(HEADER_COLOR, "Scene Input");
    commandLineTable("SceneInputOptions", {
        {"-in scene.rdla|rdlb", "Input RDL scene file (can be used multiple times)"},
        {"-deltas file.rdla|rdlb", "Apply delta updates to scene (can be used multiple times)"},
        {"-camera name", "Select which camera to render from"},
        {"-layer name", "Select which render layer to display"}
    });
    ImGui::Spacing();

    ImGui::TextColored(HEADER_COLOR, "Rendering");
    commandLineTable("RenderingOptions", {
        {"-size W H", "Canonical frame width and height in pixels"},
        {"-res divisor", "Resolution divisor (e.g., 2 for half resolution)"},
        {"-threads n", "Number of render threads (default: all available)"},
        {"-free_cam", "Use freecam mode instead of default orbit camera"},
        {"-fast_geometry_update", "Enable fast geometry updates for animation"}
    });
    ImGui::Spacing();

    ImGui::TextColored(HEADER_COLOR, "Output & Debugging");
    commandLineTable("OutputOptions", {
        {"-out filename.exr", "Output image filename and format"},
        {"-snap_path path", "Directory for snapshot files"},
        {"-debug_pixel x y", "Render only a single pixel for debugging"},
        {"-info", "Enable verbose progress and statistics logging"},
        {"-stats file.csv", "Log statistics to CSV file"}
    });
}

void
HelpWindow::drawDenoiseControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "Denoising can be toggled on/off and configured using the hotkeys below. There are multiple "
                          "denoisers available, including Optix and OIDN (CPU & CUDA). You can select which render "
                          "buffers to use for denoising as well. All hotkeys can be customized in the Key Bindings "
                          "window, and any changes will be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    if (ACTION_CATEGORIES.find("Denoising") != ACTION_CATEGORIES.end()) {
        drawHotkeyTable(ACTION_CATEGORIES.at("Denoising"), viewport, "Denoising");
    }
}

void
HelpWindow::drawFastProgressiveControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "Fast progressive rendering modes allow you to preview different aspects of the render "
                          "quickly. Toggle fast progressive rendering on/off, or cycle through the available modes "
                          "using the hotkeys below. All hotkeys can be customized in the Key Bindings window, and "
                          "any changes will be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    ImGui::TextWrapped("Available Fast Progressive Modes:");
    ImGui::BulletText("NORMALS: Normal vectors");
    ImGui::BulletText("SHADING_NORMALS: Shaded normals");
    ImGui::BulletText("FACING_RATIO: Face direction ratio");
    ImGui::BulletText("FACING_RATIO_INVERSE: Inverse face direction ratio");
    ImGui::BulletText("UVS: Texture coordinates");
    ImGui::Spacing();

    if (ACTION_CATEGORIES.find("Fast Progressive") != ACTION_CATEGORIES.end()) {
        drawHotkeyTable(ACTION_CATEGORIES.at("Fast Progressive"), viewport, "Fast Progressive");
    }
    ImGui::Spacing();
}

void
HelpWindow::drawOutputControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "Manage render outputs and save images. You can toggle the snapshot manager, take snapshots, "
                          "and navigate through previous snapshots. You can also save the render buffer to an "
                          "image file. All hotkeys can be customized in the Key Bindings window, and any changes will "
                          "be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    if (ACTION_CATEGORIES.find("Output") != ACTION_CATEGORIES.end()) {
        drawHotkeyTable(ACTION_CATEGORIES.at("Output"), viewport, "Output");
    }
}

void
HelpWindow::drawPathVisualizerControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "The Path Visualizer allows you to see the ray paths for debugging. Use the hotkeys below to "
                          "toggle the path visualizer and adjust its settings. All hotkeys can be customized in the "
                          "Key Bindings window, and any changes will be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    if (ACTION_CATEGORIES.find("Path Visualizer") != ACTION_CATEGORIES.end()) {
        drawHotkeyTable(ACTION_CATEGORIES.at("Path Visualizer"), viewport, "Path Visualizer");
    }
}

void drawStatusBarDiagram(const float boxHeight, const float lineHeight)
{
    // Draw a visual diagram of the status bar
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    const float totalWidth = ImGui::GetContentRegionAvail().x;

    // Define sections with their approximate widths (in percentages)
    struct StatusBarSection {
        const char* label;
        const char* example;
        float widthPercent;
        ImVec4 color;
    };

    const StatusBarSection sections[] = {
        {"Pixel Position", "(100, 250)", 0.12f, SECTION_COLORS[0]},
        {"Channel", "CH: RGB", 0.15f, SECTION_COLORS[1]},
        {"Fast Mode", "FAST: OFF", 0.15f, SECTION_COLORS[2]},
        {"Denoiser", "OPTIX: BEAUTY", 0.25f, SECTION_COLORS[3]},
        {"Camera", "ORBIT_CAM", 0.15f, SECTION_COLORS[4]},
        {"Render Output", "RO: beauty", 0.12f, SECTION_COLORS[5]},
        {"Help", "?", 0.06f, SECTION_COLORS[6]},
    };

    float xOffset = cursorPos.x;
    ImVec2 diagramTopLeft(xOffset, cursorPos.y);
    ImVec2 diagramBottomRight(xOffset + totalWidth, cursorPos.y + boxHeight);

    // Draw the outer border
    drawList->AddRect(diagramTopLeft, diagramBottomRight, ImGui::GetColorU32(OUTLINE_COLOR), 0.0f, 0, 2.0f);

    // Draw sections
    float currentX = xOffset + 2;
    for (const auto& section : sections) {
        float sectionWidth = totalWidth * section.widthPercent;
        ImVec2 sectionTopLeft(currentX, cursorPos.y + 2);
        ImVec2 sectionBottomRight(currentX + sectionWidth, cursorPos.y + boxHeight - 2);

        // Draw section background
        ImVec4 bgColor = ImVec4(section.color.x * 0.3f, section.color.y * 0.3f, section.color.z * 0.3f, 0.5f);
        drawList->AddRectFilled(sectionTopLeft, sectionBottomRight, ImGui::GetColorU32(bgColor));

        // Draw section border
        drawList->AddRect(sectionTopLeft, sectionBottomRight, ImGui::GetColorU32(section.color), 0.0f, 0, 1.0f);

        // Draw text inside section
        drawList->AddText(ImVec2(sectionTopLeft.x + 4, sectionTopLeft.y + 6), 
                                 ImGui::GetColorU32(section.color), section.label);
        drawList->AddText(ImVec2(sectionTopLeft.x + 4, sectionTopLeft.y + 6 + lineHeight + 2), 
                          ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), section.example);

        currentX += sectionWidth;
    }
}

void
HelpWindow::drawStatusBarInfo() const
{
    ImGui::Spacing();
    std::string infoStr = "The Status Bar at the bottom of the viewport provides real-time information about the "
                          "current state of the application. Below is a visual representation, along with a description "
                          "of the information displayed.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();
    ImGui::Spacing();

    const float lineHeight = ImGui::GetTextLineHeight();
    const float boxHeight = lineHeight * 4;
    drawStatusBarDiagram(boxHeight, lineHeight);
    
    ImGui::Dummy(ImVec2(0, boxHeight));
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Detailed explanation
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Field Descriptions:");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "Pixel Position");
    std::string pixelInfoStr = "Current mouse position in image coordinates (x, y). Updates as you move the mouse over "
                               "the rendered image.";
    ImGui::TextWrapped("%s", pixelInfoStr.c_str());
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.3f, 1.0f), "Channel (CH)");
    ImGui::TextWrapped("Indicates the current color channel being viewed. Possible values include: ");
    ImGui::BulletText("RGB: Full color view");
    ImGui::BulletText("RED/GREEN/BLUE: Single channel view");
    ImGui::BulletText("LUM: Luminance");
    ImGui::BulletText("NRGB: Normalized RGB (0-1)");
    ImGui::BulletText("SPP: Samples Per Pixel");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Fast Progressive Mode (FAST)");
    ImGui::TextWrapped("Displays the current fast progressive mode. When enabled, the renderer prioritizes speed "
                       "over quality for faster feedback during scene setup and adjustments.");
    ImGui::BulletText("OFF: Disabled");
    ImGui::BulletText("NORMALS: Normal vectors");
    ImGui::BulletText("SHADING_NORMALS: Shaded normals");
    ImGui::BulletText("FACING_RATIO: Face direction ratio");
    ImGui::BulletText("FACING_RATIO_INVERSE: Inverse face direction ratio");
    ImGui::BulletText("UVS: Texture coordinates");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.8f, 1.0f), "Denoiser");
    ImGui::TextWrapped("Shows the active denoising algorithm and buffers being used.");
    ImGui::Spacing();
    ImGui::TextWrapped("Denoising Mode:");
    ImGui::BulletText("OFF: Denoising is disabled");
    ImGui::BulletText("OPTIX: NVIDIA OptiX denoiser");
    ImGui::BulletText("OIDN: Intel Open Image Denoise");
    ImGui::BulletText("OIDN_CPU: Open Image Denoise CPU fallback");
    ImGui::BulletText("OIDN_CUDA: Open Image Denoise CUDA fallback");
    ImGui::Spacing();
    ImGui::TextWrapped("Denoising Buffer Mode (only applicable when denoising is enabled):");
    ImGui::BulletText("BEAUTY: Denoising only the beauty pass");
    ImGui::BulletText("BEAUTY_ALBEDO: Denoising beauty + albedo passes");
    ImGui::BulletText("BEAUTY_ALBEDO_NORMALS: Denoising beauty + albedo + normals passes");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.8f, 1.0f), "Camera");
    ImGui::TextWrapped("Current camera mode: ORBIT_CAM (rotate around pivot) or FREE_CAM (FPS-style movement)");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Render Output (RO)");
    ImGui::TextWrapped("Name of the current render output being displayed.");
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Help Button");
    ImGui::TextWrapped("Click the '?' button to open this help window");
}

void
HelpWindow::drawViewportControls(Viewport* viewport) const
{
    ImGui::Spacing();
    std::string infoStr = "Use the hotkeys below to control the viewport display. You can pan and zoom the image, "
                          "toggle the tile progress overlay, and more. All hotkeys can be customized in the Key "
                          "Bindings window, and any changes will be reflected here.";
    ImGui::TextWrapped("%s", infoStr.c_str());
    ImGui::Spacing();

    const std::vector<std::string> viewportCategories = {"Image Controls", "Window Toggles", "Channels"};
    for (const auto& catName : viewportCategories) {
        if (ACTION_CATEGORIES.find(catName) != ACTION_CATEGORIES.end()) {
            ImGui::TextColored(HEADER_COLOR, "%s", catName.c_str());
            drawHotkeyTable(ACTION_CATEGORIES.at(catName), viewport, catName.c_str());
            ImGui::Spacing();
        }
    }
}

} // namespace moonray_gui_v2

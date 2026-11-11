// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "PixelInspector.h"
#include "../Viewport.h"

namespace moonray_gui_v2 {

void
PixelInspector::getSubimageUVCoords(ImVec2& uv0, ImVec2& uv1, const ImVec2& currentPixel, 
                                    const float textureWidth, const float textureHeight) const
{
    int halfSizeX = mSubimageWidth / 2;
    int halfSizeY = mSubimageHeight / 2;
    float subimageX = currentPixel.x - halfSizeX;                       // pixel coord x of top left corner of subimage
    float subimageY = textureHeight - 1 - currentPixel.y - halfSizeY;   // pixel coord y of top left corner of subimage

    // UV coordinate for top left corner of subimage
    uv0 = ImVec2(subimageX / textureWidth, 1 - (subimageY / textureHeight));
    // UV coordinate for bottom right corner of subimage
    uv1 = ImVec2((subimageX + mSubimageWidth) / textureWidth, 1 - ((subimageY + mSubimageHeight) / textureHeight));
}

void
PixelInspector::getSubimagePixelRect(ImVec2& rectMin, ImVec2& rectMax, const ImVec2& subimagePixel, 
                                     const ImVec2& imageDisplaySize) const
{
    rectMin = ImGui::GetItemRectMin();

    float pixelDisplaySize = imageDisplaySize.x / mSubimageWidth;

    rectMin.x += subimagePixel.x * pixelDisplaySize;
    rectMin.y += subimagePixel.y * pixelDisplaySize;

    rectMax = rectMin;
    rectMax.x += pixelDisplaySize;
    rectMax.y += pixelDisplaySize;
}

void
PixelInspector::drawOutOfRangePixels(const ImVec2& imageDisplaySize, const ImVec2& currentPixel, 
                                      const float textureWidth, const float textureHeight)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    int halfSizeX = mSubimageWidth / 2;
    int halfSizeY = mSubimageHeight / 2;

    int pixelDisplaySize = static_cast<int>(imageDisplaySize.x / mSubimageWidth);

    for (int i = 0; i < mSubimageWidth; ++i) {
        for (int j = 0; j < mSubimageHeight; ++j) {
            float pixelX = currentPixel.x - halfSizeX + i;
            // We need to flip the y coordinate since our pixel coords have origin at 
            // bottom left, but the subimage is drawn top to bottom
            float pixelY = currentPixel.y - halfSizeY + (mSubimageHeight - 1 - j);
            // If this pixel is out of range, draw black rectangle
            if (pixelX < 0 || pixelX >= textureWidth || pixelY < 0 || pixelY >= textureHeight) {
                ImVec2 rectMin, rectMax;
                getSubimagePixelRect(rectMin, rectMax, ImVec2(i, j), imageDisplaySize);
                drawList->AddRectFilled(rectMin, rectMax, mOOBColor);
            }
        }
    }
}

void 
PixelInspector::drawOutlineAroundPixel(const ImVec2& imageDisplaySize)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Get the rectangle for the center pixel in the subimage (2,2)
    ImVec2 rectMin, rectMax;
    getSubimagePixelRect(rectMin, rectMax, ImVec2(2, 2), imageDisplaySize);

    // Draw a outline around the pixel
    drawList->AddRect(rectMin, rectMax, mPixelOutlineColor, 0.0f, 0, mPixelOutlineThickness);
}

void
PixelInspector::drawColorValue(const scene_rdl2::math::Color& color)
{
    auto drawColorText = [](float value, const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::Text("%.4f", value);
        ImGui::PopStyleColor();
    };

    // Display the pixel color values
    ImGui::Text("(");
    ImGui::SameLine(0,0);

    drawColorText(color.r, mRedColor);

    ImGui::SameLine(0,0);
    ImGui::Text(", ");
    ImGui::SameLine(0,0);

    drawColorText(color.g, mGreenColor);
    
    ImGui::SameLine(0,0);
    ImGui::Text(", ");
    ImGui::SameLine(0,0);

    drawColorText(color.b, mBlueColor);
    ImGui::SameLine(0,0);

    ImGui::Text(")");
}

void
PixelInspector::configureWindow(const ImVec2& dockOffset) const
{
    // Center the window in the viewport when first opened
    ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    // Position at bottom right with some padding from edges
    ImVec2 defaultPos = ImVec2(imguiViewport->Size.x - dockOffset.x - mPadding - mWidth, 
                               imguiViewport->Size.y - dockOffset.y - mPadding - mHeight);
    ImGui::SetNextWindowPos(defaultPos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(mWidth, mHeight), ImGuiCond_Appearing);
}

void
PixelInspector::draw(Viewport* viewport, const ImVec2& currentPixel, const ImVec2& dockOffset)
{
    if (!mOpen) { return; }

    float textureWidth = static_cast<float>(viewport->getFramebufferWidth());       // width of full framebuffer
    float textureHeight = static_cast<float>(viewport->getFramebufferHeight());     // height of full framebuffer
    GLuint textureHandle = viewport->getFramebufferTexture();            // OpenGL texture handle for framebuffer

    ImVec2 uv0, uv1;
    getSubimageUVCoords(uv0, uv1, currentPixel, textureWidth, textureHeight);

    // Size of the window to display the subimage (initial widthxwidth minus some padding)
    ImVec2 imageDisplaySize = ImVec2(mWidth - mSubimagePadding, mWidth - mSubimagePadding);

    configureWindow(dockOffset);

    // ------------ Start of window creation ---------------
    ImGui::Begin("Pixel Inspector", &mOpen);

    if (currentPixel.x >= 0 && currentPixel.y >= 0) {
        // Display the subimage
        ImGui::Image(textureHandle, imageDisplaySize, uv0, uv1);

        // Draw black pixels in areas outside of the framebuffer
        drawOutOfRangePixels(imageDisplaySize, currentPixel, textureWidth, textureHeight);
        
        // Draw outline around pixel in subimage display (the center pixel in the 5x5 subimage)
        drawOutlineAroundPixel(imageDisplaySize);

        // Get color of pixel under mouse cursor and display its value
        scene_rdl2::math::Color pixelColor = viewport->getPixelColor(currentPixel.x, currentPixel.y);
        drawColorValue(pixelColor);
    } else {
        // Draw placeholder for out-of-range pixel
        ImGui::Dummy(imageDisplaySize);
        ImGui::Text("Pixel out of range");
    }
    ImGui::End();
    // ------------ End of window creation ---------------
}

} // namespace moonray_gui_v2

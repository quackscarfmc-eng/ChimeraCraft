// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace moonray_gui_v2 {

// ----------------------------- Draw Tile Outlines -------------------------------- //
/// TODO: In the future, we may want to have imgui draw the tile outlines as an overlay
/// instead of drawing directly into the framebuffer.

// Experimental:
// Set to non-zero to only draw the corners of overlaid quads instead of the
// full square. It may be slightly less distracting.
#define DRAW_PARTIAL_TILE_OUTLINE       0

inline uint8_t
convertToByteColor(float col)
{
    return uint8_t(math::saturate(col) * 255.f);
}

inline void
addSaturate(uint8_t &fb, uint8_t c)
{
    uint8_t result = static_cast<uint8_t>(fb + c);
    fb = (result >= fb) ? result : 255;
}

inline void
addSaturate(float &fb, float c)
{
    float result = fb + c;
    fb = math::saturate(result);
}

inline void
addSaturate(scene_rdl2::fb_util::ByteColor &bc, uint8_t c)
{
    addSaturate(bc.r, c);
    addSaturate(bc.g, c);
    addSaturate(bc.b, c);
}

inline void
addSaturate(scene_rdl2::fb_util::RenderColor &rc, float c)
{
    addSaturate(rc.x, c);
    addSaturate(rc.y, c);
    addSaturate(rc.z, c);
}

inline void
addSaturate(math::Vec3f &v, float c)
{
    addSaturate(v.x, c);
    addSaturate(v.y, c);
    addSaturate(v.z, c);
}

template<typename BufferType, typename ScalarType> void 
drawHorizontalLine(BufferType *buf, unsigned x0, unsigned x1, unsigned y, const ScalarType col)
{
    auto *row = buf->getRow(y);
    for (unsigned x = x0; x < x1; ++x) {
        addSaturate(row[x], col);
    }
}

template<typename BufferType, typename ScalarType> void
drawVerticalLine(BufferType *buf, unsigned x, unsigned y0, unsigned y1, const ScalarType col)
{
    for (unsigned y = y0; y < y1; ++y) {
        auto &pixel = buf->getPixel(x, y);
        addSaturate(pixel, col);
    }
}

template<typename BufferType, typename ScalarType> void
drawFullTileOutline(BufferType *buf, const scene_rdl2::fb_util::Tile &tile, const ScalarType col)
{
    drawHorizontalLine(buf, tile.mMinX,     tile.mMaxX,     tile.mMinY,     col);
    drawHorizontalLine(buf, tile.mMinX,     tile.mMaxX,     tile.mMaxY - 1, col);
    drawVerticalLine(buf,   tile.mMinX,     tile.mMinY + 1, tile.mMaxY - 1, col);
    drawVerticalLine(buf,   tile.mMaxX - 1, tile.mMinY + 1, tile.mMaxY - 1, col);
}

template<typename BufferType, typename ScalarType> void
drawPoint(BufferType *buf, unsigned x, unsigned y, const ScalarType col)
{
    auto &pixel = buf->getPixel(x, y);
    addSaturate(pixel, col);
}

inline uint8_t
fadeColor(uint8_t c)
{
    return static_cast<uint8_t>(c >> 1);
}

inline float
fadeColor(float c)
{
    return c * 0.5f;
}

template<typename BufferType, typename ScalarType> void
drawPartialTileOutline(BufferType *buf, const scene_rdl2::fb_util::Tile &tile, const ScalarType col)
{
    ScalarType fadeCol = fadeColor(col);

    drawPoint(buf, tile.mMinX,     tile.mMinY,     col);
    drawPoint(buf, tile.mMinX + 1, tile.mMinY,     fadeCol);
    drawPoint(buf, tile.mMinX,     tile.mMinY + 1, fadeCol);
    drawPoint(buf, tile.mMinX + 2, tile.mMinY,     col);
    drawPoint(buf, tile.mMinX,     tile.mMinY + 2, col);

    drawPoint(buf, tile.mMinX,     tile.mMaxY - 1, col);
    drawPoint(buf, tile.mMinX + 1, tile.mMaxY - 1, fadeCol);
    drawPoint(buf, tile.mMinX,     tile.mMaxY - 2, fadeCol);
    drawPoint(buf, tile.mMinX + 2, tile.mMaxY - 1, col);
    drawPoint(buf, tile.mMinX,     tile.mMaxY - 3, col);

    drawPoint(buf, tile.mMaxX - 1, tile.mMinY,     col);
    drawPoint(buf, tile.mMaxX - 2, tile.mMinY,     fadeCol);
    drawPoint(buf, tile.mMaxX - 1, tile.mMinY + 1, fadeCol);
    drawPoint(buf, tile.mMaxX - 3, tile.mMinY,     col);
    drawPoint(buf, tile.mMaxX - 1, tile.mMinY + 2, col);

    drawPoint(buf, tile.mMaxX - 1, tile.mMaxY - 1, col);
    drawPoint(buf, tile.mMaxX - 2, tile.mMaxY - 1, fadeCol);
    drawPoint(buf, tile.mMaxX - 1, tile.mMaxY - 2, fadeCol);
    drawPoint(buf, tile.mMaxX - 3, tile.mMaxY - 1, col);
    drawPoint(buf, tile.mMaxX - 1, tile.mMaxY - 3, col);
}

template <typename BufferType, typename ScalarType> void
drawClippedPoint(BufferType *buf, unsigned x, unsigned y, const ScalarType col)
{
    if (x < buf->getWidth() && y < buf->getHeight()) {
        drawPoint(buf, x, y, col);
    }
}

template<typename BufferType, typename ScalarType> void
drawPartialTileOutlineClipped(BufferType *buf, const scene_rdl2::fb_util::Tile &tile, const ScalarType col)
{
    ScalarType fadeCol = fadeColor(col);

    drawClippedPoint(buf, tile.mMinX,     tile.mMinY,     col);
    drawClippedPoint(buf, tile.mMinX + 1, tile.mMinY,     col);
    drawClippedPoint(buf, tile.mMinX,     tile.mMinY + 1, col);
    drawClippedPoint(buf, tile.mMinX + 2, tile.mMinY,     fadeCol);
    drawClippedPoint(buf, tile.mMinX,     tile.mMinY + 2, fadeCol);

    drawClippedPoint(buf, tile.mMinX,     tile.mMaxY - 1, col);
    drawClippedPoint(buf, tile.mMinX + 1, tile.mMaxY - 1, col);
    drawClippedPoint(buf, tile.mMinX,     tile.mMaxY - 2, col);
    drawClippedPoint(buf, tile.mMinX + 2, tile.mMaxY - 1, fadeCol);
    drawClippedPoint(buf, tile.mMinX,     tile.mMaxY - 3, fadeCol);

    drawClippedPoint(buf, tile.mMaxX - 1, tile.mMinY,     col);
    drawClippedPoint(buf, tile.mMaxX - 2, tile.mMinY,     col);
    drawClippedPoint(buf, tile.mMaxX - 1, tile.mMinY + 1, col);
    drawClippedPoint(buf, tile.mMaxX - 3, tile.mMinY,     fadeCol);
    drawClippedPoint(buf, tile.mMaxX - 1, tile.mMinY + 2, fadeCol);

    drawClippedPoint(buf, tile.mMaxX - 1, tile.mMaxY - 1, col);
    drawClippedPoint(buf, tile.mMaxX - 2, tile.mMaxY - 1, col);
    drawClippedPoint(buf, tile.mMaxX - 1, tile.mMaxY - 2, col);
    drawClippedPoint(buf, tile.mMaxX - 3, tile.mMaxY - 1, fadeCol);
    drawClippedPoint(buf, tile.mMaxX - 1, tile.mMaxY - 3, fadeCol);
}

template<typename BufferType, typename ScalarType> void
drawTileOutline(BufferType *buf, const scene_rdl2::fb_util::Tile &tile, const ScalarType col)
{
    if (DRAW_PARTIAL_TILE_OUTLINE) {
        if (tile.getArea() == 64) {
            drawPartialTileOutline(buf, tile, col);
        } else {
            drawPartialTileOutlineClipped(buf, tile, col);
        }
    } else {
        drawFullTileOutline(buf, tile, col);
    }
}

} // end namespace moonray_gui_v2
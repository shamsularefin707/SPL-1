#pragma once

#include "Image.h"

namespace fer {

/**
 * @brief Geometry of an in-plane rotation about the image centre (used for head-roll handling).
 *
 * PURPOSE
 *   A face detector trained on upright faces misses heads that are tilted ("rolled") by more
 *   than ~10 degrees. Instead of building a rotated copy of every template and every verifier,
 *   the detector rotates the WHOLE IMAGE by a few fixed angles and runs the upright pipeline on
 *   each copy (Rowley, Baluja & Kanade 1998 use the same idea). A face tilted by +20 degrees is
 *   upright in the copy rotated by -20 degrees.
 *
 * MATHEMATICAL IDEA
 *   Image coordinates have y pointing DOWN. With theta in radians, c = cos(theta), s = sin(theta)
 *   and the rotation matrix R(theta) = [[c, -s], [s, c]]:
 *
 *       source point  p  = C  + R(theta) (p' - C')      (rotated -> source, used for sampling)
 *       rotated point p' = C' + R(-theta)(p  - C )      (source -> rotated)
 *
 *   C is the centre of the source image and C' the centre of the (larger) rotated canvas.
 *   The canvas is enlarged to  W' = |W c| + |H s|,  H' = |W s| + |H c|  so no pixel is lost.
 *
 * COMPLEXITY  O(W' * H' * channels) with one bilinear sample per output pixel.
 */
struct RotationFrame {
    double degrees{0.0};
    double cosT{1.0};
    double sinT{0.0};
    double srcCx{0.0}, srcCy{0.0};   ///< Centre C of the source image
    double dstCx{0.0}, dstCy{0.0};   ///< Centre C' of the rotated canvas
    int dstWidth{0}, dstHeight{0};   ///< Rotated canvas size

    /// Builds the frame for rotating a (srcW x srcH) image by `degrees`.
    static RotationFrame make(int srcW, int srcH, double degrees);

    /// Maps a point of the rotated canvas back to the source image.
    void toSource(double xr, double yr, double& xs, double& ys) const noexcept {
        const double dx = xr - dstCx, dy = yr - dstCy;
        xs = srcCx + cosT * dx - sinT * dy;
        ys = srcCy + sinT * dx + cosT * dy;
    }

    /// Maps a point of the source image into the rotated canvas.
    void toRotated(double xs, double ys, double& xr, double& yr) const noexcept {
        const double dx = xs - srcCx, dy = ys - srcCy;
        xr = dstCx + cosT * dx + sinT * dy;
        yr = dstCy - sinT * dx + cosT * dy;
    }
};

/**
 * @brief Rotates an image (any channel count) onto an enlarged canvas with bilinear sampling.
 *
 * @param src        Source image.
 * @param frame      Rotation frame from RotationFrame::make(src.width(), src.height(), deg).
 * @param fillValue  Value written where the canvas has no source pixel.
 * @param validMask  Optional output: 1-channel mask, 255 where the pixel came from the source,
 *                   0 in the empty corners. The detector uses it to ignore windows that fall
 *                   partly outside the real photo.
 */
Image rotateImage(const Image& src, const RotationFrame& frame, uint8_t fillValue = 0,
                  Image* validMask = nullptr);

/**
 * @brief Samples an UPRIGHT (outW x outH) patch from a rotated rectangle of the source.
 *
 * The rectangle is centred at (cx, cy), has size (rectW x rectH) in its own frame and is
 * rolled by `degrees` (same sign convention as RotationFrame). Output pixel (u, v) takes
 * the bilinear sample at  C + R(theta) * ((u + 0.5) * rectW / outW - rectW / 2, ...).
 * Pixels that fall outside the source are clamped to the border.
 *
 * Used by FaceExtractor to cut a tilted face out of a photo so that the expression
 * recogniser always receives an upright face.
 */
Image sampleRotatedRect(const Image& src, double cx, double cy, double rectW, double rectH,
                        double degrees, int outW, int outH);

} // namespace fer

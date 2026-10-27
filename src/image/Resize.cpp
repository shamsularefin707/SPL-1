#include "Resize.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace fer {

/**
 * ============================================================================
 * Mathematical and Algorithmic Documentation: 2D Image Resizing
 * ============================================================================
 *
 * 1. Coordinate Mapping (Continuous Center-Aligned Alignment):
 *    In digital raster grids, pixel (x, y) represents a continuous square area
 *    centered at (x + 0.5, y + 0.5).
 *    To map a destination pixel (xd, yd) to a continuous source coordinate (xs, ys)
 *    without geometric shift, asymmetric cropping, or edge distortion, we apply:
 *
 *        xs = (xd + 0.5) * (width_src / width_dst) - 0.5
 *        ys = (yd + 0.5) * (height_src / height_dst) - 0.5
 *
 *    We then clamp xs to [0.0, width_src - 1.0] and ys to [0.0, height_src - 1.0].
 *
 * 2. Bilinear Interpolation:
 *    Let (xs, ys) be decomposed into integer grid indices and fractional offsets:
 *        x0 = floor(xs),   x1 = min(x0 + 1, width_src - 1)
 *        y0 = floor(ys),   y1 = min(y0 + 1, height_src - 1)
 *        dx = xs - x0,     dy = ys - y0
 *
 *    Bilinear interpolation computes the intensity by taking a 2D weighted average
 *    of the 4 surrounding spatial neighbors:
 *        Q00 = I(x0, y0),   Q10 = I(x1, y0)
 *        Q01 = I(x0, y1),   Q11 = I(x1, y1)
 *
 *    Horizontal interpolation along top and bottom rows:
 *        R0 = (1.0 - dx) * Q00 + dx * Q10
 *        R1 = (1.0 - dx) * Q01 + dx * Q11
 *
 *    Vertical interpolation between R0 and R1:
 *        V  = (1.0 - dy) * R0  + dy * R1
 *           = (1-dx)(1-dy)*Q00 + dx*(1-dy)*Q10 + (1-dx)*dy*Q01 + dx*dy*Q11
 *
 *    The resulting continuous value is rounded to the nearest integer and clamped
 *    to [0, 255].
 * ============================================================================
 */

Image resize(const Image& input, int newWidth, int newHeight, InterpolationMethod method) {
    if (!input.isValid()) {
        throw std::invalid_argument("resize: Source image is empty or invalid.");
    }
    if (newWidth <= 0 || newHeight <= 0) {
        throw std::invalid_argument("resize: Target dimensions must be positive (got " +
                                     std::to_string(newWidth) + "x" + std::to_string(newHeight) + ").");
    }

    int srcW = input.width();
    int srcH = input.height();
    int channels = input.channels();

    // If dimensions already match, return a copy directly
    if (srcW == newWidth && srcH == newHeight) {
        return input;
    }

    Image output(newWidth, newHeight, channels);

    double scaleX = static_cast<double>(srcW) / static_cast<double>(newWidth);
    double scaleY = static_cast<double>(srcH) / static_cast<double>(newHeight);

    if (method == InterpolationMethod::BILINEAR) {
        for (int yd = 0; yd < newHeight; ++yd) {
            // Map destination Y to continuous source Y
            double ys = (yd + 0.5) * scaleY - 0.5;
            ys = std::clamp(ys, 0.0, static_cast<double>(srcH - 1));

            int y0 = static_cast<int>(std::floor(ys));
            int y1 = std::min(y0 + 1, srcH - 1);
            double dy = ys - y0;
            double invDy = 1.0 - dy;

            for (int xd = 0; xd < newWidth; ++xd) {
                // Map destination X to continuous source X
                double xs = (xd + 0.5) * scaleX - 0.5;
                xs = std::clamp(xs, 0.0, static_cast<double>(srcW - 1));

                int x0 = static_cast<int>(std::floor(xs));
                int x1 = std::min(x0 + 1, srcW - 1);
                double dx = xs - x0;
                double invDx = 1.0 - dx;

                // Precalculate 4 spatial weights
                double w00 = invDx * invDy;
                double w10 = dx * invDy;
                double w01 = invDx * dy;
                double w11 = dx * dy;

                for (int c = 0; c < channels; ++c) {
                    double p00 = input.getPixelFast(x0, y0, c);
                    double p10 = input.getPixelFast(x1, y0, c);
                    double p01 = input.getPixelFast(x0, y1, c);
                    double p11 = input.getPixelFast(x1, y1, c);

                    double val = w00 * p00 + w10 * p10 + w01 * p01 + w11 * p11;
                    int rounded = static_cast<int>(std::round(val));
                    output.setPixelFast(xd, yd, c, static_cast<uint8_t>(std::clamp(rounded, 0, 255)));
                }
            }
        }
    } else { // Nearest neighbor
        for (int yd = 0; yd < newHeight; ++yd) {
            int ys = static_cast<int>(std::floor((yd + 0.5) * scaleY));
            ys = std::clamp(ys, 0, srcH - 1);

            for (int xd = 0; xd < newWidth; ++xd) {
                int xs = static_cast<int>(std::floor((xd + 0.5) * scaleX));
                xs = std::clamp(xs, 0, srcW - 1);

                for (int c = 0; c < channels; ++c) {
                    output.setPixelFast(xd, yd, c, input.getPixelFast(xs, ys, c));
                }
            }
        }
    }

    return output;
}

} // namespace fer

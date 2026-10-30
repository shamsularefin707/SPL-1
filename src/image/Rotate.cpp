#include "Rotate.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fer {

namespace {

// Bilinear sample of channel c at a continuous position; returns false when (x, y) lies
// outside the image (callers decide whether to fill or clamp).
inline bool bilinear(const Image& img, double x, double y, int c, double& out) noexcept {
    const int w = img.width();
    const int h = img.height();
    if (x < -0.5 || y < -0.5 || x > w - 0.5 || y > h - 0.5) {
        return false;
    }
    x = std::clamp(x, 0.0, static_cast<double>(w - 1));
    y = std::clamp(y, 0.0, static_cast<double>(h - 1));
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = std::min(x0 + 1, w - 1);
    const int y1 = std::min(y0 + 1, h - 1);
    const double fx = x - x0;
    const double fy = y - y0;
    out = (1 - fx) * (1 - fy) * img.getPixelFast(x0, y0, c) + fx * (1 - fy) * img.getPixelFast(x1, y0, c) +
          (1 - fx) * fy * img.getPixelFast(x0, y1, c) + fx * fy * img.getPixelFast(x1, y1, c);
    return true;
}

} // namespace

RotationFrame RotationFrame::make(int srcW, int srcH, double degrees) {
    if (srcW <= 0 || srcH <= 0) {
        throw std::invalid_argument("RotationFrame::make: source size must be positive.");
    }
    RotationFrame f;
    f.degrees = degrees;
    const double rad = degrees * 3.14159265358979323846 / 180.0;
    f.cosT = std::cos(rad);
    f.sinT = std::sin(rad);
    // Exactly-upright frames keep the source size (so angle 0 is a no-op).
    if (std::abs(degrees) < 1e-9) {
        f.cosT = 1.0;
        f.sinT = 0.0;
    }
    const double ac = std::abs(f.cosT), as = std::abs(f.sinT);
    f.dstWidth = static_cast<int>(std::ceil(srcW * ac + srcH * as - 1e-9));
    f.dstHeight = static_cast<int>(std::ceil(srcW * as + srcH * ac - 1e-9));
    // Pixel-centre convention: the centre of a W-pixel row is at (W - 1) / 2.
    f.srcCx = 0.5 * (srcW - 1);
    f.srcCy = 0.5 * (srcH - 1);
    f.dstCx = 0.5 * (f.dstWidth - 1);
    f.dstCy = 0.5 * (f.dstHeight - 1);
    return f;
}

Image rotateImage(const Image& src, const RotationFrame& frame, uint8_t fillValue, Image* validMask) {
    if (!src.isValid()) {
        throw std::invalid_argument("rotateImage: invalid source image.");
    }
    const int W = frame.dstWidth;
    const int H = frame.dstHeight;
    const int C = src.channels();
    Image out(W, H, C, fillValue);
    if (validMask != nullptr) {
        *validMask = Image(W, H, 1, 0);
    }
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            double sx, sy;
            frame.toSource(x, y, sx, sy);
            bool inside = true;
            for (int c = 0; c < C && inside; ++c) {
                double v;
                inside = bilinear(src, sx, sy, c, v);
                if (inside) {
                    out.setPixelFast(x, y, c, static_cast<uint8_t>(std::clamp(std::lround(v), 0L, 255L)));
                }
            }
            if (inside && validMask != nullptr) {
                validMask->setPixelFast(x, y, 0, 255);
            }
        }
    }
    return out;
}

Image sampleRotatedRect(const Image& src, double cx, double cy, double rectW, double rectH,
                        double degrees, int outW, int outH) {
    if (!src.isValid() || outW <= 0 || outH <= 0 || rectW <= 0.0 || rectH <= 0.0) {
        throw std::invalid_argument("sampleRotatedRect: invalid arguments.");
    }
    const double rad = degrees * 3.14159265358979323846 / 180.0;
    const double c = std::cos(rad), s = std::sin(rad);
    const int C = src.channels();
    Image out(outW, outH, C, 0);
    for (int v = 0; v < outH; ++v) {
        const double dy = (v + 0.5) * rectH / outH - 0.5 * rectH;
        for (int u = 0; u < outW; ++u) {
            const double dx = (u + 0.5) * rectW / outW - 0.5 * rectW;
            // Rectangle frame -> source: p = centre + R(theta) * (dx, dy)
            double sx = cx + c * dx - s * dy;
            double sy = cy + s * dx + c * dy;
            sx = std::clamp(sx, 0.0, static_cast<double>(src.width() - 1));
            sy = std::clamp(sy, 0.0, static_cast<double>(src.height() - 1));
            for (int ch = 0; ch < C; ++ch) {
                double val = 0.0;
                bilinear(src, sx, sy, ch, val);
                out.setPixelFast(u, v, ch, static_cast<uint8_t>(std::clamp(std::lround(val), 0L, 255L)));
            }
        }
    }
    return out;
}

} // namespace fer

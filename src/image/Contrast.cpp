#include "Contrast.h"
#include <cmath>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace fer {

/**
 * ============================================================================
 * Mathematical and Algorithmic Documentation: Contrast Stretching
 * ============================================================================
 *
 * Contrast stretching (normalization) expands the dynamic range of pixel
 * intensities to span the full available range [0, 255].
 *
 * For specified thresholds [low, high]:
 *   - Any pixel intensity at or below 'low' is clamped to 0 (pure black).
 *   - Any pixel intensity at or above 'high' is clamped to 255 (pure white).
 *   - Any intermediate intensity 'v' is linearly scaled:
 *
 *               v - low
 *       v_out = -------- * 255
 *              high - low
 *
 * Implementation Optimization:
 * Since input pixel values are discrete 8-bit unsigned integers in [0, 255],
 * we compute a 256-element Lookup Table (LUT) once. This eliminates floating-point
 * divisions inside the nested pixel iteration loops, executing in O(W * H) with
 * single-cycle array lookups.
 *
 * Edge Case Handling:
 *   1. low > high:
 *      Illogical parameterization. Throws std::invalid_argument.
 *   2. low == high:
 *      Degenerate thresholding. Pixels strictly less than 'low' are mapped to 0,
 *      and pixels greater than or equal to 'high' are mapped to 255.
 * ============================================================================
 */

Image contrastStretch(const Image& input, uint8_t low, uint8_t high) {
    if (!input.isValid()) {
        throw std::invalid_argument("contrastStretch: Input image is empty or invalid.");
    }

    if (low > high) {
        throw std::invalid_argument("contrastStretch: Lower bound (" + std::to_string(low) +
                                     ") cannot exceed upper bound (" + std::to_string(high) + ").");
    }

    // Build 256-entry Lookup Table
    std::array<uint8_t, 256> lut;

    if (low == high) {
        // Degenerate step function
        for (int v = 0; v < 256; ++v) {
            lut[v] = (v < low) ? 0 : 255;
        }
    } else {
        double range = static_cast<double>(high - low);
        for (int v = 0; v < 256; ++v) {
            if (v <= low) {
                lut[v] = 0;
            } else if (v >= high) {
                lut[v] = 255;
            } else {
                double scaled = ((v - low) / range) * 255.0;
                int rounded = static_cast<int>(std::round(scaled));
                lut[v] = static_cast<uint8_t>(std::clamp(rounded, 0, 255));
            }
        }
    }

    // Apply LUT across the image
    int width = input.width();
    int height = input.height();
    int channels = input.channels();

    Image output(width, height, channels);
    size_t totalBytes = input.totalBytes();
    const uint8_t* inPtr = input.data();
    uint8_t* outPtr = output.data();

    for (size_t i = 0; i < totalBytes; ++i) {
        outPtr[i] = lut[inPtr[i]];
    }

    return output;
}

Image autoContrastStretch(const Image& input) {
    if (!input.isValid()) {
        throw std::invalid_argument("autoContrastStretch: Input image is empty or invalid.");
    }

    size_t totalBytes = input.totalBytes();
    const uint8_t* data = input.data();

    uint8_t minVal = 255;
    uint8_t maxVal = 0;

    for (size_t i = 0; i < totalBytes; ++i) {
        if (data[i] < minVal) minVal = data[i];
        if (data[i] > maxVal) maxVal = data[i];
    }

    // If constant image, stretching is not possible
    if (minVal == maxVal) {
        return input;
    }

    return contrastStretch(input, minVal, maxVal);
}

Image percentileContrastStretch(const Image& input, double lowPct, double highPct) {
    if (!input.isValid()) {
        throw std::invalid_argument("percentileContrastStretch: Input image is empty or invalid.");
    }

    // Handle degenerate / invalid percentile ranges gracefully
    if (lowPct >= highPct || lowPct < 0.0 || highPct > 100.0) {
        return input;
    }

    size_t totalBytes = input.totalBytes();
    if (totalBytes == 0) {
        return input;
    }

    const uint8_t* data = input.data();

    // Compute 256-bin histogram
    std::array<size_t, 256> hist = {0};
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
    for (size_t i = 0; i < totalBytes; ++i) {
        uint8_t val = data[i];
        hist[val]++;
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    // Handle constant image
    if (minVal == maxVal) {
        return input;
    }

    double lowTarget = (lowPct / 100.0) * static_cast<double>(totalBytes);
    double highTarget = (highPct / 100.0) * static_cast<double>(totalBytes);

    uint8_t lowVal = minVal;
    uint8_t highVal = maxVal;
    size_t cum = 0;
    bool foundLow = false;

    for (int v = 0; v < 256; ++v) {
        cum += hist[v];
        if (!foundLow && static_cast<double>(cum) >= lowTarget) {
            lowVal = static_cast<uint8_t>(v);
            foundLow = true;
        }
        if (static_cast<double>(cum) >= highTarget) {
            highVal = static_cast<uint8_t>(v);
            break;
        }
    }

    if (lowVal >= highVal) {
        return input;
    }

    return contrastStretch(input, lowVal, highVal);
}

} // namespace fer


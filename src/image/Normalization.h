#pragma once

#include "Image.h"
#include <cstdint>

namespace fer {

/// Standard facial expression recognition canonical image dimensions defined by the research paper.
constexpr int FER_TARGET_WIDTH = 132;
constexpr int FER_TARGET_HEIGHT = 132;

/**
 * @brief Executes the research preprocessing pipeline for Facial Expression Recognition (Tawhid et al. 2012).
 *
 * Pipeline sequence:
 *   1. Grayscale Conversion: (R + G + B) / 3
 *   2. Resizing: Bilinear interpolation to 132 x 132 pixels
 *   3. Percentile Contrast Stretching: Paper Eq. 3 (default: lowPct = 1.0, highPct = 99.0)
 *   4. Histogram Equalization: CDF-based global contrast redistribution
 *
 * @param input Raw input image (color or grayscale).
 * @param lowPct Lower percentile bound (default: 1.0).
 * @param highPct Upper percentile bound (default: 99.0).
 * @return Fully normalized 132 x 132 single-channel FER face image.
 * @throws std::invalid_argument if input image is empty or invalid.
 */
Image preprocessForFER(
    const Image& input,
    double lowPct = 1.0,
    double highPct = 99.0
);

/**
 * @brief Legacy overload supporting manual threshold or auto min/max contrast stretching.
 *
 * @param input Raw input image (color or grayscale).
 * @param contrastLow Lower threshold for contrast stretching.
 * @param contrastHigh Upper threshold for contrast stretching.
 * @param autoContrast If true, uses autoContrastStretch() based on image min/max intensities.
 * @return Fully normalized 132 x 132 single-channel FER face image.
 */
Image preprocessForFER(
    const Image& input,
    uint8_t contrastLow,
    uint8_t contrastHigh,
    bool autoContrast = false
);

} // namespace fer

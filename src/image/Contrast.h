#pragma once

#include "Image.h"
#include <cstdint>

namespace fer {

/**
 * @brief Linearly stretches the contrast of an image between specified low and high thresholds.
 *
 * Mathematical mapping:
 *   - If pixel <= low: mapped to 0
 *   - If pixel >= high: mapped to 255
 *   - If low < pixel < high: mapped to round( (pixel - low) / (high - low) * 255.0 )
 *
 * Edge cases:
 *   - low > high: Throws std::invalid_argument.
 *   - low == high: Degenerate binary thresholding: pixels < low are 0, pixels >= high are 255.
 *
 * @param input The source image (grayscale or multichannel).
 * @param low Lower intensity bound.
 * @param high Upper intensity bound.
 * @return Contrast-stretched Image.
 * @throws std::invalid_argument if low > high or input image is invalid.
 */
Image contrastStretch(const Image& input, uint8_t low, uint8_t high);

/**
 * @brief Automatically finds the minimum and maximum pixel intensities in the image
 *        and performs contrast stretching across the full [0, 255] dynamic range.
 *
 * If the image is uniform (min == max), returns a copy of the input.
 *
 * @param input The source image.
 * @return Contrast-stretched Image.
 */
Image autoContrastStretch(const Image& input);

/**
 * @brief Performs contrast stretching using percentile cutoffs computed from the image histogram (paper Eq. 3).
 *
 * Discards outlier pixel intensities below lowPct and above highPct before linearly
 * expanding the remaining dynamic range to [0, 255].
 *
 * Edge cases:
 *   - Constant/uniform image (all pixels equal): returns a copy of input.
 *   - lowPct >= highPct: returns a copy of input.
 *   - lowVal == highVal: returns a copy of input.
 *
 * @param input The source image (grayscale or multichannel).
 * @param lowPct Lower percentile bound (default: 1.0).
 * @param highPct Upper percentile bound (default: 99.0).
 * @return Contrast-stretched Image.
 * @throws std::invalid_argument if input image is empty or invalid.
 */
Image percentileContrastStretch(const Image& input, double lowPct = 1.0, double highPct = 99.0);

} // namespace fer


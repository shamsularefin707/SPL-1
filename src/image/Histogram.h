#pragma once

#include "Image.h"
#include <vector>
#include <cstdint>

namespace fer {

/**
 * @brief Calculates an intensity histogram for an 8-bit single-channel (grayscale) image.
 *
 * Produces exactly 256 bins, where bin index 'i' holds the total frequency of pixels
 * with intensity value 'i'.
 *
 * Mathematical constraint:
 *   sum(histogram[0..255]) == image.width() * image.height()
 *
 * @param image The input single-channel image.
 * @return A 256-element vector containing pixel counts per intensity bin.
 * @throws std::invalid_argument if image is not 1-channel or is invalid.
 */
std::vector<uint32_t> calculateHistogram(const Image& image);

/**
 * @brief Computes the discrete Cumulative Distribution Function (CDF) of a histogram.
 *
 * CDF[v] = sum_{j=0}^{v} histogram[j]
 *
 * @param histogram A 256-bin histogram vector.
 * @return A 256-element cumulative sum vector.
 * @throws std::invalid_argument if histogram size is not 256.
 */
std::vector<uint32_t> calculateCDF(const std::vector<uint32_t>& histogram);

/**
 * @brief Performs histogram equalization on a single-channel grayscale image.
 *
 * Spreads out frequent intensity values across the entire [0, 255] spectrum
 * using the normalized cumulative distribution function (CDF).
 *
 * Mathematical formulation:
 *   LUT[v] = round( ((CDF[v] - CDF_min) / (total_pixels - CDF_min)) * 255.0 )
 *
 * Robust edge case handling:
 *   - Constant/uniform image (all pixels same value): detects total_pixels == CDF_min
 *     and safely preserves pixel values without division-by-zero.
 *   - Absent intensity levels: correctly handled by non-decreasing CDF mapping.
 *
 * @param input Single-channel grayscale image.
 * @return Histogram-equalized single-channel Image.
 * @throws std::invalid_argument if input is not 1-channel or is empty.
 */
Image equalizeHistogram(const Image& input);

} // namespace fer

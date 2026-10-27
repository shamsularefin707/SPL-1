#pragma once

#include "Image.h"

namespace fer {

/**
 * @brief Resizing interpolation methods.
 */
enum class InterpolationMethod {
    BILINEAR,          ///< Bilinear interpolation (continuous weighted average of 4 neighbors)
    NEAREST_NEIGHBOR   ///< Nearest neighbor interpolation (fastest, discrete)
};

/**
 * @brief Resizes an image to arbitrary target dimensions (newWidth x newHeight).
 *
 * Employs center-aligned continuous coordinate mapping and 2D bilinear interpolation
 * across all channels.
 *
 * Target FER preprocessing uses 132 x 132 pixels, but this function is fully generic.
 *
 * @param input The source image to resize.
 * @param newWidth Target width in pixels (must be > 0).
 * @param newHeight Target height in pixels (must be > 0).
 * @param method Interpolation algorithm (defaults to BILINEAR).
 * @return Resized Image with dimensions (newWidth x newHeight x input.channels()).
 * @throws std::invalid_argument if dimensions are <= 0 or source image is empty.
 */
Image resize(const Image& input, int newWidth, int newHeight,
             InterpolationMethod method = InterpolationMethod::BILINEAR);

} // namespace fer

#pragma once

#include "Image.h"

namespace fer {

/**
 * @brief Converts a color or multi-channel image to a 1-channel grayscale image.
 *
 * Implements the exact arithmetic average formula used in the research paper:
 *   Gray = (R + G + B) / 3
 *
 * @param input Input image (1, 3, or 4 channels).
 * @return 1-channel grayscale Image with identical spatial dimensions (width x height).
 * @throws std::invalid_argument if input image is empty or invalid.
 */
Image toGrayscale(const Image& input);

} // namespace fer

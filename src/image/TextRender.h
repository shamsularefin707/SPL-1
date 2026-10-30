#pragma once

#include "Image.h"
#include <string>
#include <cstdint>

namespace fer {

/**
 * @brief From-scratch 5x7 bitmap text for annotating result images (no font library).
 *
 * Each character is 5 columns of 7 bits (bit 0 = top row), the classic LCD font layout.
 * Supported: A-Z (lower case is drawn as upper case), 0-9, space and % . : - ( ) /
 * Unknown characters are drawn as a blank.
 *
 * @param scale   Every font pixel becomes a (scale x scale) block.
 * @param filled  Draw a solid background box (bgR, bgG, bgB) behind the text first.
 * Text that runs off the image is clipped, never an error.
 */
void drawText(Image& image, int x, int y, const std::string& text, int scale,
              uint8_t r, uint8_t g, uint8_t b,
              bool filled = false, uint8_t bgR = 0, uint8_t bgG = 0, uint8_t bgB = 0);

/// Width in pixels of `text` drawn at `scale` (6 columns per character incl. spacing).
int textWidth(const std::string& text, int scale);

/// Height in pixels of one line drawn at `scale` (7 rows).
int textHeight(int scale);

} // namespace fer

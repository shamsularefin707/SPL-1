#include "Grayscale.h"

namespace fer {

/**
 * ============================================================================
 * Theoretical and Architectural Documentation: Grayscale Conversion
 * ============================================================================
 *
 * 1. Why RGB images have three channels:
 *    - Human color vision is trichromatic, mediated by three types of cone
 *      photoreceptors in the retina (S, M, and L cones), sensitive to short
 *      (blue ~420-440nm), medium (green ~530-540nm), and long (red ~560-580nm)
 *      wavelengths.
 *    - Standard digital color imaging captures and reproduces this spectrum using
 *      three additive primary channels: Red (R), Green (G), and Blue (B).
 *      Each pixel stores a triplet (R, G, B), spanning a 24-bit color gamut
 *      with 16,777,216 distinct color combinations.
 *
 * 2. Why grayscale has one channel:
 *    - Grayscale images discard chromatic information (hue and saturation) and
 *      retain only scalar luminous intensity (luminance/brightness).
 *    - In Facial Expression Recognition (FER), emotional expressions are defined
 *      by physical muscular deformations: eyebrow lowering, mouth widening,
 *      lip corner depression, and skin wrinkling. These geometric and textural
 *      cues are conveyed through edges, shadows, and luminance gradients.
 *    - Chromaticity introduces non-expressive confounding factors such as
 *      skin pigmentation variations, cosmetics, and illumination color tints.
 *    - Reducing to a single channel eliminates 66.7% of redundant data, speeding up
 *      subsequent face detection, resizing, contrast stretching, and PCA projections.
 *
 * 3. How the grayscale value is calculated:
 *    - In accordance with the project's research paper preprocessing methodology:
 *          Gray = (R + G + B) / 3
 *    - We compute this using integer arithmetic:
 *          gray = static_cast<uint8_t>((static_cast<uint32_t>(r) + g + b) / 3);
 *      The intermediate sum is cast to 32-bit unsigned integer to prevent
 *      overflow (since 255 * 3 = 765 > 255), and divided by 3, yielding an
 *      intensity strictly in the range [0, 255].
 *
 * 4. What happens to the image dimensions:
 *    - Spatial dimensions (width and height) remain invariant:
 *          width_out  = width_in
 *          height_out = height_in
 *    - Depth (channel count) decreases:
 *          channels_out = 1
 *    - Memory footprint is reduced to exactly 1/3 of the RGB input buffer size.
 * ============================================================================
 */

Image toGrayscale(const Image& input) {
    if (!input.isValid()) {
        throw std::invalid_argument("toGrayscale: Input image is empty or invalid.");
    }

    int width = input.width();
    int height = input.height();
    int channels = input.channels();

    // If already 1-channel grayscale, return a direct copy
    if (channels == 1) {
        return input;
    }

    Image grayImage(width, height, 1);

    if (channels == 3 || channels == 4) {
        // Fast processing across rows and columns
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint32_t r = input.getPixelFast(x, y, 0);
                uint32_t g = input.getPixelFast(x, y, 1);
                uint32_t b = input.getPixelFast(x, y, 2);

                // Arithmetic average formula: Gray = (R + G + B) / 3
                uint8_t gray = static_cast<uint8_t>((r + g + b) / 3);

                grayImage.setPixelFast(x, y, 0, gray);
            }
        }
    } else {
        throw std::invalid_argument("toGrayscale: Unsupported number of channels (" +
                                     std::to_string(channels) + "). Expected 1, 3, or 4.");
    }

    return grayImage;
}

} // namespace fer

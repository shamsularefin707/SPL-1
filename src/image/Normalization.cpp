#include "Normalization.h"
#include "Grayscale.h"
#include "Resize.h"
#include "Contrast.h"
#include "Histogram.h"
#include <stdexcept>

namespace fer {

Image preprocessForFER(
    const Image& input,
    double lowPct,
    double highPct
) {
    if (!input.isValid()) {
        throw std::invalid_argument("preprocessForFER: Input image is empty or invalid.");
    }

    // Step 1: Convert to single-channel Grayscale using (R + G + B) / 3
    Image gray = toGrayscale(input);

    // Step 2: Resize to canonical 132 x 132 resolution using Bilinear Interpolation
    Image resized = resize(gray, FER_TARGET_WIDTH, FER_TARGET_HEIGHT, InterpolationMethod::BILINEAR);

    // Step 3: Percentile Contrast Stretching (Paper Eq. 3)
    Image contrasted = percentileContrastStretch(resized, lowPct, highPct);

    // Step 4: Histogram Equalization for lighting normalization
    Image normalized = equalizeHistogram(contrasted);

    return normalized;
}

Image preprocessForFER(
    const Image& input,
    uint8_t contrastLow,
    uint8_t contrastHigh,
    bool autoContrast
) {
    if (!input.isValid()) {
        throw std::invalid_argument("preprocessForFER: Input image is empty or invalid.");
    }

    // Step 1: Convert to single-channel Grayscale using (R + G + B) / 3
    Image gray = toGrayscale(input);

    // Step 2: Resize to canonical 132 x 132 resolution using Bilinear Interpolation
    Image resized = resize(gray, FER_TARGET_WIDTH, FER_TARGET_HEIGHT, InterpolationMethod::BILINEAR);

    // Step 3: Contrast Stretching
    Image contrasted = autoContrast
        ? autoContrastStretch(resized)
        : contrastStretch(resized, contrastLow, contrastHigh);

    // Step 4: Histogram Equalization for lighting normalization
    Image normalized = equalizeHistogram(contrasted);

    return normalized;
}

} // namespace fer

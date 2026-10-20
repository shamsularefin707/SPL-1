#include "Histogram.h"
#include <cmath>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace fer {

/**
 * ============================================================================
 * Mathematical and Algorithmic Documentation: Histogram & Equalization
 * ============================================================================
 *
 * 1. Grayscale Histogram:
 *    For an 8-bit discrete image I(x, y) with spatial size W x H:
 *        H(k) = sum_{x=0}^{W-1} sum_{y=0}^{H-1} [ I(x, y) == k ],  for k in [0, 255]
 *    The sum over all bins satisfies:
 *        sum_{k=0}^{255} H(k) = W * H = total_pixels
 *
 * 2. Cumulative Distribution Function (CDF):
 *    CDF(k) = sum_{j=0}^{k} H(j)
 *    Properties:
 *        - CDF(k) is monotonically non-decreasing.
 *        - CDF(255) == total_pixels.
 *
 * 3. Histogram Equalization:
 *    The continuous ideal seeks a transformation T(r) such that the output
 *    probability density function P_s(s) is uniform across [0, L-1] (L=256).
 *    In discrete digital domains, the transformation is given by:
 *
 *                  CDF(k) - CDF_min
 *        LUT(k) = ----------------- * (L - 1)
 *                     N - CDF_min
 *
 *    where:
 *        N       = total number of pixels (W * H)
 *        CDF_min = minimum non-zero cumulative distribution value
 *        L       = 256 (for 8-bit images, L - 1 = 255)
 *
 * 4. Division-by-Zero and Edge Case Safeguards:
 *    - If all pixels in the image possess the identical intensity value, then:
 *          CDF_min == N
 *      resulting in a denominator of zero (N - CDF_min = 0).
 *      In this degenerate case, division by zero is strictly guarded against;
 *      the function returns a copy of the input image unchanged.
 * ============================================================================
 */

std::vector<uint32_t> calculateHistogram(const Image& image) {
    if (!image.isValid()) {
        throw std::invalid_argument("calculateHistogram: Image is empty or invalid.");
    }
    if (image.channels() != 1) {
        throw std::invalid_argument("calculateHistogram: Histogram calculation requires a 1-channel grayscale image (received " +
                                     std::to_string(image.channels()) + " channels).");
    }

    std::vector<uint32_t> hist(256, 0);
    size_t total = image.totalPixels();
    const uint8_t* ptr = image.data();

    for (size_t i = 0; i < total; ++i) {
        hist[ptr[i]]++;
    }

    return hist;
}

std::vector<uint32_t> calculateCDF(const std::vector<uint32_t>& histogram) {
    if (histogram.size() != 256) {
        throw std::invalid_argument("calculateCDF: Expected 256-bin histogram, but received " +
                                     std::to_string(histogram.size()) + " bins.");
    }

    std::vector<uint32_t> cdf(256, 0);
    cdf[0] = histogram[0];
    for (size_t i = 1; i < 256; ++i) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    return cdf;
}

Image equalizeHistogram(const Image& input) {
    if (!input.isValid()) {
        throw std::invalid_argument("equalizeHistogram: Image is empty or invalid.");
    }
    if (input.channels() != 1) {
        throw std::invalid_argument("equalizeHistogram: Histogram equalization requires a 1-channel grayscale image (received " +
                                     std::to_string(input.channels()) + " channels).");
    }

    size_t totalPixels = input.totalPixels();
    if (totalPixels == 0) {
        return input;
    }

    // 1. Calculate histogram
    std::vector<uint32_t> hist = calculateHistogram(input);

    // 2. Calculate cumulative distribution function (CDF)
    std::vector<uint32_t> cdf = calculateCDF(hist);

    // 3. Find minimum non-zero value in CDF
    uint32_t cdfMin = 0;
    for (int i = 0; i < 256; ++i) {
        if (cdf[i] > 0) {
            cdfMin = cdf[i];
            break;
        }
    }

    // 4. Handle constant/uniform image safeguard:
    // If all pixels have the exact same intensity, cdfMin == totalPixels.
    // Denominator (totalPixels - cdfMin) becomes 0.
    if (cdfMin >= totalPixels) {
        return input; // No dynamic range to equalize
    }

    // 5. Construct normalized intensity lookup table (LUT)
    std::array<uint8_t, 256> lut;
    double denominator = static_cast<double>(totalPixels - cdfMin);

    for (int i = 0; i < 256; ++i) {
        if (cdf[i] < cdfMin) {
            lut[i] = 0;
        } else {
            double normalized = ((cdf[i] - cdfMin) / denominator) * 255.0;
            int rounded = static_cast<int>(std::round(normalized));
            lut[i] = static_cast<uint8_t>(std::clamp(rounded, 0, 255));
        }
    }

    // 6. Apply LUT to every pixel
    Image output(input.width(), input.height(), 1);
    const uint8_t* inPtr = input.data();
    uint8_t* outPtr = output.data();

    for (size_t i = 0; i < totalPixels; ++i) {
        outPtr[i] = lut[inPtr[i]];
    }

    return output;
}

} // namespace fer

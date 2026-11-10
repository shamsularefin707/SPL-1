#include "image/ImageProcessing.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

/**
 * @brief Creates a synthetic face-like RGB test image with distinct features and gradients.
 *
 * Used for autonomous demonstration when no external BMP image is provided.
 */
fer::Image createSyntheticFacePattern(int width = 200, int height = 240) {
    fer::Image img(width, height, 3);

    int cx = width / 2;
    int cy = height / 2;
    int radius = std::min(width, height) * 4 / 10;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Background: smooth diagonal pastel gradient
            uint8_t bgR = static_cast<uint8_t>(40 + (x * 40) / width);
            uint8_t bgG = static_cast<uint8_t>(50 + (y * 50) / height);
            uint8_t bgB = static_cast<uint8_t>(80 + ((x + y) * 40) / (width + height));

            img.setPixelFast(x, y, 0, bgR);
            img.setPixelFast(x, y, 1, bgG);
            img.setPixelFast(x, y, 2, bgB);

            // Face ellipse
            double dx = (x - cx) / static_cast<double>(radius * 0.85);
            double dy = (y - cy) / static_cast<double>(radius);
            if (dx * dx + dy * dy <= 1.0) {
                // Skin-like tone with radial shading
                double dist = std::sqrt(dx * dx + dy * dy);
                uint8_t skinR = static_cast<uint8_t>(std::clamp(210 - dist * 40.0, 0.0, 255.0));
                uint8_t skinG = static_cast<uint8_t>(std::clamp(170 - dist * 40.0, 0.0, 255.0));
                uint8_t skinB = static_cast<uint8_t>(std::clamp(140 - dist * 30.0, 0.0, 255.0));

                img.setPixelFast(x, y, 0, skinR);
                img.setPixelFast(x, y, 1, skinG);
                img.setPixelFast(x, y, 2, skinB);

                // Left Eye
                int eyeY = cy - radius / 3;
                int leftEyeX = cx - radius / 3;
                int rightEyeX = cx + radius / 3;
                int eyeRadius = radius / 7;

                int dLeftEyeX = x - leftEyeX;
                int dLeftEyeY = y - eyeY;
                if (dLeftEyeX * dLeftEyeX + dLeftEyeY * dLeftEyeY <= eyeRadius * eyeRadius) {
                    img.setPixelFast(x, y, 0, 30);
                    img.setPixelFast(x, y, 1, 30);
                    img.setPixelFast(x, y, 2, 45);
                }

                // Right Eye
                int dRightEyeX = x - rightEyeX;
                int dRightEyeY = y - eyeY;
                if (dRightEyeX * dRightEyeX + dRightEyeY * dRightEyeY <= eyeRadius * eyeRadius) {
                    img.setPixelFast(x, y, 0, 30);
                    img.setPixelFast(x, y, 1, 30);
                    img.setPixelFast(x, y, 2, 45);
                }

                // Mouth (Smiling curve)
                int mouthY = cy + radius / 2;
                if (y >= mouthY - 3 && y <= mouthY + 5 && std::abs(x - cx) <= radius / 3) {
                    double offset = std::abs(x - cx) / static_cast<double>(radius / 3);
                    int expectedY = mouthY + static_cast<int>(offset * offset * 8.0);
                    if (std::abs(y - expectedY) <= 2) {
                        img.setPixelFast(x, y, 0, 160);
                        img.setPixelFast(x, y, 1, 50);
                        img.setPixelFast(x, y, 2, 50);
                    }
                }
            }
        }
    }
    return img;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n";
    std::cout << "  FER Project: Module 1 (Image Processing Demonstration)\n";
    std::cout << "========================================================\n\n";

    std::string inputPath = "sample_input.bmp";

    if (argc > 1) {
        inputPath = argv[1];
        std::cout << "[Step 1] Loading user-provided image: " << inputPath << "\n";
    } else {
        std::cout << "[Step 1] Generating synthetic face pattern -> " << inputPath << "\n";
        fer::Image synthetic = createSyntheticFacePattern(200, 240);
        fer::ImageLoader::saveBMP(inputPath, synthetic);
        std::cout << "         Synthetic BMP generated (" << synthetic.width() << "x"
                  << synthetic.height() << ", " << synthetic.channels() << " channels)\n";
    }

    try {
        // 1. Load BMP
        fer::Image input = fer::ImageLoader::loadBMP(inputPath);
        std::cout << "  -> Loaded successfully!\n";
        std::cout << "     Dimensions: " << input.width() << " x " << input.height() << "\n";
        std::cout << "     Channels:   " << input.channels() << "\n";
        std::cout << "     Total Bytes:" << input.totalBytes() << " bytes\n\n";

        // 2. Grayscale Conversion
        std::cout << "[Step 2] Converting to Grayscale using formula: Gray = (R + G + B) / 3\n";
        fer::Image gray = fer::toGrayscale(input);
        std::cout << "     Output Dimensions: " << gray.width() << " x " << gray.height() << "\n";
        std::cout << "     Output Channels:   " << gray.channels() << " (single-channel)\n";
        fer::ImageLoader::saveBMP("demo_1_grayscale.bmp", gray);
        std::cout << "  -> Saved intermediate: demo_1_grayscale.bmp\n\n";

        // 3. Image Resizing
        std::cout << "[Step 3] Resizing to research target 132 x 132 using Bilinear Interpolation\n";
        fer::Image resized = fer::resize(gray, fer::FER_TARGET_WIDTH, fer::FER_TARGET_HEIGHT,
                                         fer::InterpolationMethod::BILINEAR);
        std::cout << "     Output Dimensions: " << resized.width() << " x " << resized.height() << "\n";
        std::cout << "     Output Channels:   " << resized.channels() << "\n";
        fer::ImageLoader::saveBMP("demo_2_resized_132x132.bmp", resized);
        std::cout << "  -> Saved intermediate: demo_2_resized_132x132.bmp\n\n";

        // 4. Contrast Stretching
        std::cout << "[Step 4] Applying Contrast Stretching (Auto Min-Max dynamic expansion)\n";
        fer::Image contrasted = fer::autoContrastStretch(resized);
        std::cout << "     Processed " << contrasted.totalPixels() << " pixels across full [0, 255] spectrum\n";
        fer::ImageLoader::saveBMP("demo_3_contrast_stretched.bmp", contrasted);
        std::cout << "  -> Saved intermediate: demo_3_contrast_stretched.bmp\n\n";

        // 5. Histogram Calculation & Histogram Equalization
        std::cout << "[Step 5] Calculating Histogram and performing Histogram Equalization\n";
        std::vector<uint32_t> histBefore = fer::calculateHistogram(contrasted);
        uint32_t sumHist = 0;
        for (uint32_t count : histBefore) sumHist += count;
        std::cout << "     Histogram bins: 256 | Total counted pixels: " << sumHist
                  << " (Expected: " << contrasted.totalPixels() << ")\n";

        fer::Image equalized = fer::equalizeHistogram(contrasted);
        std::cout << "     Equalization complete! Dimensions: " << equalized.width() << " x "
                  << equalized.height() << "\n";
        fer::ImageLoader::saveBMP("demo_4_histogram_equalized.bmp", equalized);
        std::cout << "  -> Saved intermediate: demo_4_histogram_equalized.bmp\n\n";

        // 6. Complete End-to-End FER Preprocessing Pipeline
        std::cout << "[Step 6] Running complete preprocessForFER() pipeline\n";
        fer::Image finalFER = fer::preprocessForFER(input, 0, 255, true);
        fer::ImageLoader::saveBMP("demo_5_pipeline_result.bmp", finalFER);
        std::cout << "     Pipeline Result: " << finalFER.width() << "x" << finalFER.height()
                  << " (" << finalFER.channels() << " channel)\n";
        std::cout << "  -> Saved final result: demo_5_pipeline_result.bmp\n\n";

        std::cout << "========================================================\n";
        std::cout << "  Demonstration Finished Successfully!\n";
        std::cout << "  Output files generated in current directory:\n";
        std::cout << "    1. sample_input.bmp\n";
        std::cout << "    2. demo_1_grayscale.bmp\n";
        std::cout << "    3. demo_2_resized_132x132.bmp\n";
        std::cout << "    4. demo_3_contrast_stretched.bmp\n";
        std::cout << "    5. demo_4_histogram_equalized.bmp\n";
        std::cout << "    6. demo_5_pipeline_result.bmp\n";
        std::cout << "========================================================\n";

    } catch (const std::exception& ex) {
        std::cerr << "\n[ERROR]: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

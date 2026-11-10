#include "image/ImageProcessing.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <filesystem>

// Simple assertion macro with diagnostic output
#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAIL: " << msg << " (line " << __LINE__ << ")\n"; \
            std::exit(1); \
        } \
    } while (0)

namespace fs = std::filesystem;

void testImageBasics() {
    std::cout << "[TEST] Image class basics & memory management... ";

    // Default constructor
    fer::Image emptyImg;
    TEST_ASSERT(emptyImg.empty(), "Empty image should report empty");
    TEST_ASSERT(!emptyImg.isValid(), "Empty image should not be valid");
    TEST_ASSERT(emptyImg.width() == 0, "Width should be 0");
    TEST_ASSERT(emptyImg.height() == 0, "Height should be 0");

    // Sized constructor
    fer::Image img(10, 20, 3, 128);
    TEST_ASSERT(!img.empty(), "Sized image should not be empty");
    TEST_ASSERT(img.isValid(), "Sized image should be valid");
    TEST_ASSERT(img.width() == 10, "Width should be 10");
    TEST_ASSERT(img.height() == 20, "Height should be 20");
    TEST_ASSERT(img.channels() == 3, "Channels should be 3");
    TEST_ASSERT(img.totalPixels() == 200, "Total pixels should be 200");
    TEST_ASSERT(img.totalBytes() == 600, "Total bytes should be 600");
    TEST_ASSERT(img.getPixel(5, 5, 0) == 128, "Initial value should be 128");

    // Pixel access & bounds checking
    img.setPixel(3, 4, 1, 240);
    TEST_ASSERT(img.getPixel(3, 4, 1) == 240, "Pixel set/get should match");

    bool threw = false;
    try {
        img.getPixel(10, 5, 0); // Out of bounds X
    } catch (const std::out_of_range&) {
        threw = true;
    }
    TEST_ASSERT(threw, "Out-of-bounds X should throw std::out_of_range");

    threw = false;
    try {
        img.getPixel(5, 20, 0); // Out of bounds Y
    } catch (const std::out_of_range&) {
        threw = true;
    }
    TEST_ASSERT(threw, "Out-of-bounds Y should throw std::out_of_range");

    threw = false;
    try {
        img.getPixel(5, 5, 3); // Out of bounds channel
    } catch (const std::out_of_range&) {
        threw = true;
    }
    TEST_ASSERT(threw, "Out-of-bounds channel should throw std::out_of_range");

    // Deep copy test
    fer::Image copyImg = img;
    copyImg.setPixel(3, 4, 1, 10);
    TEST_ASSERT(img.getPixel(3, 4, 1) == 240, "Original should remain unchanged after mutating copy");
    TEST_ASSERT(copyImg.getPixel(3, 4, 1) == 10, "Copy should reflect mutated pixel");

    // Move semantics test
    fer::Image movedImg = std::move(copyImg);
    TEST_ASSERT(movedImg.width() == 10 && movedImg.height() == 20, "Moved image should retain dimensions");
    TEST_ASSERT(movedImg.getPixel(3, 4, 1) == 10, "Moved image should retain pixels");
    TEST_ASSERT(copyImg.empty(), "Source of move should be empty");

    std::cout << "PASSED\n";
}

void testBMPLoadingAndSaving() {
    std::cout << "[TEST] BMP loader, saver, and padding handling... ";

    std::string testDir = "temp_test_bmp";
    fs::create_directories(testDir);

    // Test with non-multiple-of-4 width to strictly exercise row padding logic:
    // Width 7 * 3 bpp = 21 bytes -> padded to 24 bytes (3 padding bytes per row)
    int w = 7;
    int h = 9;
    fer::Image rgbImg(w, h, 3);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            rgbImg.setPixelFast(x, y, 0, static_cast<uint8_t>((x * 30) % 256));
            rgbImg.setPixelFast(x, y, 1, static_cast<uint8_t>((y * 25) % 256));
            rgbImg.setPixelFast(x, y, 2, static_cast<uint8_t>((x + y * 10) % 256));
        }
    }

    std::string rgbPath = testDir + "/test_rgb7x9.bmp";
    fer::ImageLoader::saveBMP(rgbPath, rgbImg);
    TEST_ASSERT(fer::ImageLoader::isBMP(rgbPath), "Saved file should be recognized as BMP");

    fer::Image loadedRGB = fer::ImageLoader::loadBMP(rgbPath);
    TEST_ASSERT(loadedRGB.width() == w, "Loaded RGB width should match");
    TEST_ASSERT(loadedRGB.height() == h, "Loaded RGB height should match");
    TEST_ASSERT(loadedRGB.channels() == 3, "Loaded RGB channels should match");

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            TEST_ASSERT(loadedRGB.getPixel(x, y, 0) == rgbImg.getPixel(x, y, 0), "RGB channel 0 mismatch");
            TEST_ASSERT(loadedRGB.getPixel(x, y, 1) == rgbImg.getPixel(x, y, 1), "RGB channel 1 mismatch");
            TEST_ASSERT(loadedRGB.getPixel(x, y, 2) == rgbImg.getPixel(x, y, 2), "RGB channel 2 mismatch");
        }
    }

    // Test 1-channel Grayscale BMP (Width 5 -> 5 bytes per row -> padded to 8 bytes)
    int gw = 5;
    int gh = 7;
    fer::Image grayImg(gw, gh, 1);
    for (int y = 0; y < gh; ++y) {
        for (int x = 0; x < gw; ++x) {
            grayImg.setPixelFast(x, y, 0, static_cast<uint8_t>(x * 40 + y * 10));
        }
    }

    std::string grayPath = testDir + "/test_gray5x7.bmp";
    fer::ImageLoader::saveBMP(grayPath, grayImg);
    fer::Image loadedGray = fer::ImageLoader::loadBMP(grayPath);
    TEST_ASSERT(loadedGray.width() == gw, "Loaded Gray width should match");
    TEST_ASSERT(loadedGray.height() == gh, "Loaded Gray height should match");
    TEST_ASSERT(loadedGray.channels() == 1, "Loaded Gray channels should be 1");

    for (int y = 0; y < gh; ++y) {
        for (int x = 0; x < gw; ++x) {
            TEST_ASSERT(loadedGray.getPixel(x, y) == grayImg.getPixel(x, y), "Gray pixel mismatch");
        }
    }

    // Clean up temporary test files
    fs::remove_all(testDir);

    std::cout << "PASSED\n";
}

void testGrayscaleConversion() {
    std::cout << "[TEST] Grayscale conversion formula (R+G+B)/3... ";

    fer::Image rgb(3, 1, 3);
    // Pixel 0: Pure Red (255, 0, 0) -> (255+0+0)/3 = 85
    rgb.setPixel(0, 0, 0, 255);
    rgb.setPixel(0, 0, 1, 0);
    rgb.setPixel(0, 0, 2, 0);

    // Pixel 1: (100, 150, 200) -> (100+150+200)/3 = 450/3 = 150
    rgb.setPixel(1, 0, 0, 100);
    rgb.setPixel(1, 0, 1, 150);
    rgb.setPixel(1, 0, 2, 200);

    // Pixel 2: (10, 20, 31) -> (10+20+31)/3 = 61/3 = 20
    rgb.setPixel(2, 0, 0, 10);
    rgb.setPixel(2, 0, 1, 20);
    rgb.setPixel(2, 0, 2, 31);

    fer::Image gray = fer::toGrayscale(rgb);
    TEST_ASSERT(gray.channels() == 1, "Grayscale image must have 1 channel");
    TEST_ASSERT(gray.width() == 3, "Width must be invariant");
    TEST_ASSERT(gray.height() == 1, "Height must be invariant");

    TEST_ASSERT(gray.getPixel(0, 0) == 85, "Pixel 0 grayscale mismatch");
    TEST_ASSERT(gray.getPixel(1, 0) == 150, "Pixel 1 grayscale mismatch");
    TEST_ASSERT(gray.getPixel(2, 0) == 20, "Pixel 2 grayscale mismatch");

    // Idempotency: Grayscale on 1-channel image returns identical image
    fer::Image grayAgain = fer::toGrayscale(gray);
    TEST_ASSERT(grayAgain.channels() == 1, "Should remain 1 channel");
    TEST_ASSERT(grayAgain.getPixel(1, 0) == 150, "Pixel value should be identical");

    std::cout << "PASSED\n";
}

void testResizing() {
    std::cout << "[TEST] Bilinear Resizing & boundary clamping... ";

    // 2x2 image:
    // [  0, 100 ]
    // [ 200, 255 ]
    fer::Image src(2, 2, 1);
    src.setPixel(0, 0, 0);
    src.setPixel(1, 0, 100);
    src.setPixel(0, 1, 200);
    src.setPixel(1, 1, 255);

    // Resize to 4x4
    fer::Image dst = fer::resize(src, 4, 4, fer::InterpolationMethod::BILINEAR);
    TEST_ASSERT(dst.width() == 4, "Resized width should be 4");
    TEST_ASSERT(dst.height() == 4, "Resized height should be 4");
    TEST_ASSERT(dst.channels() == 1, "Resized channels should be 1");

    // Values should be monotonic along rows and columns
    for (int y = 0; y < 4; ++y) {
        for (int x = 1; x < 4; ++x) {
            TEST_ASSERT(dst.getPixel(x, y) >= dst.getPixel(x - 1, y),
                        "Horizontal monotonicity violated");
        }
    }
    for (int x = 0; x < 4; ++x) {
        for (int y = 1; y < 4; ++y) {
            TEST_ASSERT(dst.getPixel(x, y) >= dst.getPixel(x, y - 1),
                        "Vertical monotonicity violated");
        }
    }

    // Downsampling test: 100x100 to 132x132 (upscaling) and 200x200 to 132x132 (downscaling)
    fer::Image large(200, 200, 3, 50);
    fer::Image downscaled = fer::resize(large, 132, 132);
    TEST_ASSERT(downscaled.width() == 132, "Downscaled width should be 132");
    TEST_ASSERT(downscaled.height() == 132, "Downscaled height should be 132");
    TEST_ASSERT(downscaled.channels() == 3, "Downscaled channels should be preserved");

    std::cout << "PASSED\n";
}

void testContrastStretching() {
    std::cout << "[TEST] Contrast stretching & boundary cases... ";

    fer::Image img(5, 1, 1);
    img.setPixel(0, 0, 50);  // <= low
    img.setPixel(1, 0, 100); // intermediate
    img.setPixel(2, 0, 150); // intermediate
    img.setPixel(3, 0, 200); // >= high
    img.setPixel(4, 0, 20);  // < low

    // Low = 50, High = 200
    fer::Image stretched = fer::contrastStretch(img, 50, 200);

    TEST_ASSERT(stretched.getPixel(0, 0) == 0, "Value at low threshold must map to 0");
    TEST_ASSERT(stretched.getPixel(4, 0) == 0, "Value below low threshold must clamp to 0");
    TEST_ASSERT(stretched.getPixel(3, 0) == 255, "Value at high threshold must map to 255");

    // Intermediate: (100 - 50) / 150 * 255 = 50 / 150 * 255 = 85
    TEST_ASSERT(stretched.getPixel(1, 0) == 85, "Intermediate value mapping mismatch");
    // Intermediate: (150 - 50) / 150 * 255 = 100 / 150 * 255 = 170
    TEST_ASSERT(stretched.getPixel(2, 0) == 170, "Intermediate value mapping mismatch");

    // Test low == high
    fer::Image step = fer::contrastStretch(img, 100, 100);
    TEST_ASSERT(step.getPixel(0, 0) == 0, "Val < 100 should map to 0");
    TEST_ASSERT(step.getPixel(1, 0) == 255, "Val >= 100 should map to 255");
    TEST_ASSERT(step.getPixel(2, 0) == 255, "Val >= 100 should map to 255");

    // Test low > high (should throw std::invalid_argument)
    bool caught = false;
    try {
        fer::contrastStretch(img, 200, 50);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    TEST_ASSERT(caught, "low > high must throw std::invalid_argument");

    // Test autoContrastStretch
    fer::Image autoStretched = fer::autoContrastStretch(img);
    // Min was 20, Max was 200
    TEST_ASSERT(autoStretched.getPixel(4, 0) == 0, "Min pixel should stretch to 0");
    TEST_ASSERT(autoStretched.getPixel(3, 0) == 255, "Max pixel should stretch to 255");

    std::cout << "PASSED\n";
}

void testPercentileContrastStretch() {
    std::cout << "[TEST] Percentile contrast stretching (paper Eq. 3)... ";

    // 1. Ramp image: 100 pixels with values 0 to 99
    fer::Image img(100, 1, 1);
    for (int i = 0; i < 100; ++i) {
        img.setPixelFast(i, 0, 0, static_cast<uint8_t>(i));
    }

    // Stretch with 5th to 95th percentile
    fer::Image pctStretched = fer::percentileContrastStretch(img, 5.0, 95.0);
    TEST_ASSERT(pctStretched.isValid(), "Output must be valid");
    // Pixels at or below 5th percentile (indices 0..4) should clamp to 0
    TEST_ASSERT(pctStretched.getPixel(0, 0) == 0, "Lower percentile pixels must clamp to 0");
    TEST_ASSERT(pctStretched.getPixel(4, 0) == 0, "5th percentile cutoff pixel must map to 0");
    // Pixels at or above 95th percentile (indices 94..99) should clamp to 255
    TEST_ASSERT(pctStretched.getPixel(94, 0) == 255, "95th percentile cutoff pixel must map to 255");
    TEST_ASSERT(pctStretched.getPixel(99, 0) == 255, "Upper percentile pixels must clamp to 255");

    // 2. Constant image: all pixels = 120
    fer::Image uniform(50, 50, 1, 120);
    fer::Image uniformStretched = fer::percentileContrastStretch(uniform, 1.0, 99.0);
    TEST_ASSERT(uniformStretched.isValid(), "Uniform output must be valid");
    TEST_ASSERT(uniformStretched.getPixel(0, 0) == 120, "Constant image must be safely preserved without error");

    // 3. Degenerate bounds (lowPct >= highPct)
    fer::Image degen = fer::percentileContrastStretch(img, 95.0, 5.0);
    TEST_ASSERT(degen.isValid(), "Degenerate bounds must safely return valid image");
    TEST_ASSERT(degen.getPixel(50, 0) == img.getPixel(50, 0), "Degenerate bounds should return unchanged copy");

    // 4. Color image test
    fer::Image colorImg(20, 20, 3, 100);
    colorImg.setPixel(0, 0, 0, 10);
    colorImg.setPixel(19, 19, 2, 240);
    fer::Image colorPct = fer::percentileContrastStretch(colorImg, 1.0, 99.0);
    TEST_ASSERT(colorPct.channels() == 3, "Color channel count must be preserved");

    std::cout << "PASSED\n";
}

void testHistogramAndEqualization() {
    std::cout << "[TEST] Histogram calculation & equalization... ";

    int w = 16;
    int h = 16;
    fer::Image img(w, h, 1);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.setPixelFast(x, y, 0, static_cast<uint8_t>((x * 16 + y) % 256));
        }
    }

    std::vector<uint32_t> hist = fer::calculateHistogram(img);
    TEST_ASSERT(hist.size() == 256, "Histogram must have 256 bins");

    uint32_t sum = 0;
    for (uint32_t count : hist) {
        sum += count;
    }
    TEST_ASSERT(sum == static_cast<uint32_t>(w * h), "Sum of histogram bins must equal total pixels");

    // Histogram equalization
    fer::Image eq = fer::equalizeHistogram(img);
    TEST_ASSERT(eq.width() == w && eq.height() == h, "Dimensions must be preserved");
    TEST_ASSERT(eq.channels() == 1, "Equalized image must be 1 channel");

    // Output values must be in [0, 255]
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t val = eq.getPixelFast(x, y, 0);
            TEST_ASSERT(val <= 255, "Pixel value must be valid 8-bit");
        }
    }

    // Critical Edge Case: Constant/Uniform Image (division-by-zero check)
    fer::Image uniform(20, 20, 1, 100);
    fer::Image eqUniform = fer::equalizeHistogram(uniform);
    TEST_ASSERT(eqUniform.width() == 20 && eqUniform.height() == 20, "Uniform dimensions preserved");
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 20; ++x) {
            TEST_ASSERT(eqUniform.getPixel(x, y) == 100, "Uniform image should safely preserve values");
        }
    }

    std::cout << "PASSED\n";
}

void testFERPreprocessingPipeline() {
    std::cout << "[TEST] End-to-end FER preprocessing pipeline... ";

    // Test with arbitrary dimension RGB input (e.g. 180 x 220)
    int inW = 180;
    int inH = 220;
    fer::Image input(inW, inH, 3);
    for (int y = 0; y < inH; ++y) {
        for (int x = 0; x < inW; ++x) {
            input.setPixelFast(x, y, 0, static_cast<uint8_t>((x * 2) % 256));
            input.setPixelFast(x, y, 1, static_cast<uint8_t>((y * 3) % 256));
            input.setPixelFast(x, y, 2, static_cast<uint8_t>((x + y) % 256));
        }
    }

    // 1. Default overload: uses percentile contrast stretch (paper Eq. 3)
    fer::Image defaultResult = fer::preprocessForFER(input);
    TEST_ASSERT(defaultResult.width() == fer::FER_TARGET_WIDTH, "FER target width must be 132");
    TEST_ASSERT(defaultResult.height() == fer::FER_TARGET_HEIGHT, "FER target height must be 132");
    TEST_ASSERT(defaultResult.channels() == 1, "FER target channels must be 1");
    TEST_ASSERT(defaultResult.isValid(), "Output image must be valid");

    // 2. Legacy overload: manual thresholds
    fer::Image result = fer::preprocessForFER(input, 10, 240, false);
    TEST_ASSERT(result.width() == fer::FER_TARGET_WIDTH, "FER target width must be 132");
    TEST_ASSERT(result.height() == fer::FER_TARGET_HEIGHT, "FER target height must be 132");
    TEST_ASSERT(result.channels() == 1, "FER target channels must be 1");
    TEST_ASSERT(result.totalPixels() == 132 * 132, "Total pixels must be 17424");
    TEST_ASSERT(result.isValid(), "Output image must be valid");

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  Running Image Processing Module Test Suite\n";
    std::cout << "========================================================\n";

    testImageBasics();
    testBMPLoadingAndSaving();
    testGrayscaleConversion();
    testResizing();
    testContrastStretching();
    testPercentileContrastStretch();
    testHistogramAndEqualization();
    testFERPreprocessingPipeline();

    std::cout << "========================================================\n";
    std::cout << "  ALL UNIT TESTS PASSED SUCCESSFULLY! (8/8 suites)\n";
    std::cout << "========================================================\n";

    return 0;
}

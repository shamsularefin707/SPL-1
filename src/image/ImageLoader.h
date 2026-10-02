#pragma once

#include "Image.h"
#include <string>

namespace fer {

/**
 * @brief Standalone loader and saver for bitmap (BMP) images.
 *
 * Implemented entirely from scratch using the C++ standard library without
 * any third-party dependencies (no OpenCV, stb_image, etc.).
 *
 * Supports:
 *  - Uncompressed 24-bit RGB BMP (BI_RGB)
 *  - Uncompressed 8-bit Grayscale/Indexed BMP (BI_RGB)
 *  - Uncompressed 32-bit RGBA BMP (BI_RGB)
 *  - Top-down and bottom-up row ordering
 *  - 4-byte scanline row padding
 *  - BGR <-> RGB channel reordering
 *
 * Explicitly rejects non-BMP formats (PNG, JPEG, etc.) and unsupported compression modes.
 */
class ImageLoader {
public:
    /**
     * @brief Loads an image from a BMP file.
     * @param filepath Path to the .bmp file.
     * @return Loaded Image object.
     * @throws std::runtime_error if file cannot be opened, is malformed, or is in an unsupported format.
     */
    static Image loadBMP(const std::string& filepath);

    /**
     * @brief Saves an Image to disk in BMP format.
     *
     * 1-channel images are written as 8-bit grayscale BMPs with an identity color palette.
     * 3-channel images are written as standard 24-bit BGR uncompressed BMPs.
     * 4-channel images are written as 32-bit BGRA uncompressed BMPs.
     *
     * @param filepath Target file path.
     * @param image The Image to save.
     * @throws std::runtime_error if the image cannot be written or dimensions are invalid.
     */
    static void saveBMP(const std::string& filepath, const Image& image);

    /**
     * @brief Helper to check whether a file appears to be a valid BMP by checking its magic bytes.
     */
    static bool isBMP(const std::string& filepath);
};

} // namespace fer

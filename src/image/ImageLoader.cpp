#include "ImageLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cmath>

namespace fer {

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t bfType{0x4D42};      // 'BM' in little-endian
    uint32_t bfSize{0};           // Size of the file in bytes
    uint16_t bfReserved1{0};
    uint16_t bfReserved2{0};
    uint32_t bfOffBits{54};       // Offset to start of pixel data
};

struct BMPInfoHeader {
    uint32_t biSize{40};          // Header size (40 bytes for BITMAPINFOHEADER)
    int32_t  biWidth{0};          // Width in pixels
    int32_t  biHeight{0};         // Height in pixels (positive: bottom-up; negative: top-down)
    uint16_t biPlanes{1};         // Number of color planes (must be 1)
    uint16_t biBitCount{24};      // Bits per pixel (e.g. 8, 24, 32)
    uint32_t biCompression{0};   // 0 = BI_RGB (uncompressed)
    uint32_t biSizeImage{0};      // Image size in bytes
    int32_t  biXPelsPerMeter{2835}; // ~72 DPI
    int32_t  biYPelsPerMeter{2835}; // ~72 DPI
    uint32_t biClrUsed{0};        // Number of colors in palette
    uint32_t biClrImportant{0};   // 0 means all colors are important
};

struct RGBQuad {
    uint8_t rgbBlue{0};
    uint8_t rgbGreen{0};
    uint8_t rgbRed{0};
    uint8_t rgbReserved{0};
};
#pragma pack(pop)

bool ImageLoader::isBMP(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    uint8_t magic[2];
    if (!file.read(reinterpret_cast<char*>(magic), 2)) {
        return false;
    }
    return magic[0] == 'B' && magic[1] == 'M';
}

Image ImageLoader::loadBMP(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("ImageLoader: Unable to open file: " + filepath);
    }

    // Read and validate file header
    BMPFileHeader fileHeader;
    if (!file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader))) {
        throw std::runtime_error("ImageLoader: Failed to read BMP file header from: " + filepath);
    }

    // Check magic bytes
    if (fileHeader.bfType != 0x4D42) {
        // Inspect magic bytes for helpful error diagnostics
        uint8_t b0 = static_cast<uint8_t>(fileHeader.bfType & 0xFF);
        uint8_t b1 = static_cast<uint8_t>((fileHeader.bfType >> 8) & 0xFF);

        if (b0 == 0x89 && b1 == 'P') {
            throw std::runtime_error("ImageLoader: File is a PNG image (" + filepath +
                                     "). Only BMP format is natively supported.");
        } else if (b0 == 0xFF && b1 == 0xD8) {
            throw std::runtime_error("ImageLoader: File is a JPEG image (" + filepath +
                                     "). Only BMP format is natively supported.");
        } else {
            throw std::runtime_error("ImageLoader: Not a valid BMP file (invalid signature 0x" +
                                     std::to_string(fileHeader.bfType) + ") in: " + filepath);
        }
    }

    // Read DIB header
    BMPInfoHeader infoHeader;
    if (!file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader))) {
        throw std::runtime_error("ImageLoader: Failed to read DIB info header from: " + filepath);
    }

    // Check header size (must be at least 40 bytes for BITMAPINFOHEADER)
    if (infoHeader.biSize < 40) {
        throw std::runtime_error("ImageLoader: Unsupported DIB header size (" +
                                 std::to_string(infoHeader.biSize) + " bytes). Must be >= 40 bytes.");
    }

    // If DIB header is larger than 40 bytes (e.g. BITMAPV4HEADER, BITMAPV5HEADER), skip the extra bytes
    if (infoHeader.biSize > 40) {
        file.seekg(infoHeader.biSize - 40, std::ios::cur);
    }

    // Check compression: allow uncompressed BI_RGB (0) and standard 32-bit BI_BITFIELDS (3)
    if (infoHeader.biCompression != 0 && !(infoHeader.biBitCount == 32 && infoHeader.biCompression == 3)) {
        throw std::runtime_error("ImageLoader: Compressed BMPs are not supported (biCompression = " +
                                 std::to_string(infoHeader.biCompression) + "). Only uncompressed BI_RGB/BI_BITFIELDS are supported.");
    }

    // Check planes
    if (infoHeader.biPlanes != 1) {
        throw std::runtime_error("ImageLoader: Unsupported number of planes (" +
                                 std::to_string(infoHeader.biPlanes) + "). Expected 1.");
    }

    // Extract width and height
    int width = infoHeader.biWidth;
    int rawHeight = infoHeader.biHeight;
    if (width <= 0) {
        throw std::runtime_error("ImageLoader: Invalid image width (" + std::to_string(width) + ").");
    }
    if (rawHeight == 0) {
        throw std::runtime_error("ImageLoader: Invalid image height (0).");
    }

    // In BMP, positive height indicates bottom-up order; negative height indicates top-down order
    bool bottomUp = (rawHeight > 0);
    int height = std::abs(rawHeight);
    uint16_t bpp = infoHeader.biBitCount;

    if (bpp != 8 && bpp != 24 && bpp != 32) {
        throw std::runtime_error("ImageLoader: Unsupported bit depth (" + std::to_string(bpp) +
                                 " bpp). Only 8, 24, and 32 bpp BMPs are supported.");
    }

    // If 8-bit, read color palette
    std::vector<RGBQuad> palette;
    bool isGrayscalePalette = true;
    if (bpp == 8) {
        uint32_t numColors = infoHeader.biClrUsed;
        if (numColors == 0) {
            numColors = 256;
        }
        palette.resize(numColors);
        if (!file.read(reinterpret_cast<char*>(palette.data()), numColors * sizeof(RGBQuad))) {
            throw std::runtime_error("ImageLoader: Failed to read color palette for 8-bit BMP.");
        }

        // Check if palette is grayscale (R == G == B)
        for (uint32_t i = 0; i < numColors; ++i) {
            if (palette[i].rgbRed != palette[i].rgbGreen || palette[i].rgbRed != palette[i].rgbBlue) {
                isGrayscalePalette = false;
                break;
            }
        }
    }

    // Seek to pixel data offset
    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (!file.good()) {
        throw std::runtime_error("ImageLoader: Failed to seek to pixel data offset (" +
                                 std::to_string(fileHeader.bfOffBits) + ").");
    }

    // Calculate row stride with 4-byte padding:
    // In BMP, each scanline row must be padded to a multiple of 4 bytes (32 bits).
    size_t rowStride = ((static_cast<size_t>(width) * bpp + 31) / 32) * 4;

    std::vector<uint8_t> rowBuffer(rowStride);

    if (bpp == 8 && isGrayscalePalette) {
        // Output 1-channel grayscale Image
        Image result(width, height, 1);

        for (int row = 0; row < height; ++row) {
            // Determine destination Y coordinate in image
            int targetY = bottomUp ? (height - 1 - row) : row;

            if (!file.read(reinterpret_cast<char*>(rowBuffer.data()), rowStride)) {
                throw std::runtime_error("ImageLoader: Corrupted pixel data at row " + std::to_string(row));
            }

            for (int x = 0; x < width; ++x) {
                uint8_t paletteIndex = rowBuffer[x];
                uint8_t grayValue = (paletteIndex < palette.size()) ? palette[paletteIndex].rgbRed : paletteIndex;
                result.setPixelFast(x, targetY, 0, grayValue);
            }
        }
        return result;
    } else if (bpp == 8) {
        // 8-bit indexed with non-grayscale palette: convert to 3-channel RGB Image
        Image result(width, height, 3);

        for (int row = 0; row < height; ++row) {
            int targetY = bottomUp ? (height - 1 - row) : row;

            if (!file.read(reinterpret_cast<char*>(rowBuffer.data()), rowStride)) {
                throw std::runtime_error("ImageLoader: Corrupted pixel data at row " + std::to_string(row));
            }

            for (int x = 0; x < width; ++x) {
                uint8_t paletteIndex = rowBuffer[x];
                if (paletteIndex < palette.size()) {
                    result.setPixelFast(x, targetY, 0, palette[paletteIndex].rgbRed);
                    result.setPixelFast(x, targetY, 1, palette[paletteIndex].rgbGreen);
                    result.setPixelFast(x, targetY, 2, palette[paletteIndex].rgbBlue);
                } else {
                    result.setPixelFast(x, targetY, 0, 0);
                    result.setPixelFast(x, targetY, 1, 0);
                    result.setPixelFast(x, targetY, 2, 0);
                }
            }
        }
        return result;
    } else if (bpp == 24) {
        // 24-bit RGB (stored as BGR in BMP)
        Image result(width, height, 3);

        for (int row = 0; row < height; ++row) {
            int targetY = bottomUp ? (height - 1 - row) : row;

            if (!file.read(reinterpret_cast<char*>(rowBuffer.data()), rowStride)) {
                throw std::runtime_error("ImageLoader: Corrupted pixel data at row " + std::to_string(row));
            }

            for (int x = 0; x < width; ++x) {
                size_t offset = static_cast<size_t>(x) * 3;
                uint8_t b = rowBuffer[offset];
                uint8_t g = rowBuffer[offset + 1];
                uint8_t r = rowBuffer[offset + 2];

                result.setPixelFast(x, targetY, 0, r);
                result.setPixelFast(x, targetY, 1, g);
                result.setPixelFast(x, targetY, 2, b);
            }
        }
        return result;
    } else { // bpp == 32
        // 32-bit RGBA (stored as BGRA in BMP)
        Image result(width, height, 4);

        for (int row = 0; row < height; ++row) {
            int targetY = bottomUp ? (height - 1 - row) : row;

            if (!file.read(reinterpret_cast<char*>(rowBuffer.data()), rowStride)) {
                throw std::runtime_error("ImageLoader: Corrupted pixel data at row " + std::to_string(row));
            }

            for (int x = 0; x < width; ++x) {
                size_t offset = static_cast<size_t>(x) * 4;
                uint8_t b = rowBuffer[offset];
                uint8_t g = rowBuffer[offset + 1];
                uint8_t r = rowBuffer[offset + 2];
                uint8_t a = rowBuffer[offset + 3];

                result.setPixelFast(x, targetY, 0, r);
                result.setPixelFast(x, targetY, 1, g);
                result.setPixelFast(x, targetY, 2, b);
                result.setPixelFast(x, targetY, 3, a);
            }
        }
        return result;
    }
}

void ImageLoader::saveBMP(const std::string& filepath, const Image& image) {
    if (!image.isValid()) {
        throw std::runtime_error("ImageLoader: Cannot save invalid or empty image to " + filepath);
    }

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("ImageLoader: Unable to open file for writing: " + filepath);
    }

    int width = image.width();
    int height = image.height();
    int channels = image.channels();

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height; // Positive indicates standard bottom-up format
    infoHeader.biPlanes = 1;
    infoHeader.biCompression = 0; // BI_RGB (uncompressed)
    infoHeader.biXPelsPerMeter = 2835;
    infoHeader.biYPelsPerMeter = 2835;

    if (channels == 1) {
        // Save as standard 8-bit grayscale BMP with 256-color linear palette
        uint16_t bpp = 8;
        infoHeader.biBitCount = bpp;
        infoHeader.biClrUsed = 256;
        infoHeader.biClrImportant = 256;

        size_t rowStride = ((static_cast<size_t>(width) * bpp + 31) / 32) * 4;
        size_t padding = rowStride - width;
        uint32_t imageSize = static_cast<uint32_t>(rowStride * height);
        infoHeader.biSizeImage = imageSize;

        uint32_t paletteSize = 256 * sizeof(RGBQuad);
        fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize;
        fileHeader.bfSize = fileHeader.bfOffBits + imageSize;

        // Write file header and info header
        file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BMPFileHeader));
        file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));

        // Write 256 grayscale palette entries: B=i, G=i, R=i, Reserved=0
        std::vector<RGBQuad> palette(256);
        for (int i = 0; i < 256; ++i) {
            palette[i].rgbBlue = static_cast<uint8_t>(i);
            palette[i].rgbGreen = static_cast<uint8_t>(i);
            palette[i].rgbRed = static_cast<uint8_t>(i);
            palette[i].rgbReserved = 0;
        }
        file.write(reinterpret_cast<const char*>(palette.data()), paletteSize);

        // Write pixel rows in bottom-up order with padding
        std::vector<uint8_t> paddingBytes(padding, 0);
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                uint8_t gray = image.getPixelFast(x, y, 0);
                file.put(static_cast<char>(gray));
            }
            if (padding > 0) {
                file.write(reinterpret_cast<const char*>(paddingBytes.data()), padding);
            }
        }
    } else if (channels == 3) {
        // Save as 24-bit BGR BMP
        uint16_t bpp = 24;
        infoHeader.biBitCount = bpp;
        infoHeader.biClrUsed = 0;
        infoHeader.biClrImportant = 0;

        size_t rowStride = ((static_cast<size_t>(width) * bpp + 31) / 32) * 4;
        size_t padding = rowStride - (static_cast<size_t>(width) * 3);
        uint32_t imageSize = static_cast<uint32_t>(rowStride * height);
        infoHeader.biSizeImage = imageSize;

        fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
        fileHeader.bfSize = fileHeader.bfOffBits + imageSize;

        file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BMPFileHeader));
        file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));

        std::vector<uint8_t> paddingBytes(padding, 0);
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                uint8_t r = image.getPixelFast(x, y, 0);
                uint8_t g = image.getPixelFast(x, y, 1);
                uint8_t b = image.getPixelFast(x, y, 2);

                // Write BGR order
                file.put(static_cast<char>(b));
                file.put(static_cast<char>(g));
                file.put(static_cast<char>(r));
            }
            if (padding > 0) {
                file.write(reinterpret_cast<const char*>(paddingBytes.data()), padding);
            }
        }
    } else if (channels == 4) {
        // Save as 32-bit BGRA BMP
        uint16_t bpp = 32;
        infoHeader.biBitCount = bpp;
        infoHeader.biClrUsed = 0;
        infoHeader.biClrImportant = 0;

        size_t rowStride = static_cast<size_t>(width) * 4; // Already 4-byte aligned
        uint32_t imageSize = static_cast<uint32_t>(rowStride * height);
        infoHeader.biSizeImage = imageSize;

        fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
        fileHeader.bfSize = fileHeader.bfOffBits + imageSize;

        file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BMPFileHeader));
        file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));

        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                uint8_t r = image.getPixelFast(x, y, 0);
                uint8_t g = image.getPixelFast(x, y, 1);
                uint8_t b = image.getPixelFast(x, y, 2);
                uint8_t a = image.getPixelFast(x, y, 3);

                file.put(static_cast<char>(b));
                file.put(static_cast<char>(g));
                file.put(static_cast<char>(r));
                file.put(static_cast<char>(a));
            }
        }
    } else {
        throw std::runtime_error("ImageLoader: Unsupported channel count (" +
                                 std::to_string(channels) + ") for BMP export.");
    }
}

} // namespace fer

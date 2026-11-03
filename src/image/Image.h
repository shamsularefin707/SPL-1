#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <stdexcept>

namespace fer {

/**
 * @brief Represents an uncompressed 2D image with 8-bit unsigned integer channels.
 *
 * Supports arbitrary width, height, and channel count (e.g. 1 for grayscale, 3 for RGB).
 * Pixel memory is stored in row-major order: index = (y * width + x) * channels + c.
 *
 * Designed from scratch without external libraries (no OpenCV cv::Mat).
 */
class Image {
public:
    /// Default constructor creating an empty image.
    Image();

    /// Constructs an image with specified dimensions and initial pixel value.
    Image(int width, int height, int channels, uint8_t initialValue = 0);

    /// Constructs an image from an existing pixel buffer (copying data).
    Image(int width, int height, int channels, const std::vector<uint8_t>& data);

    /// Constructs an image from an existing pixel buffer (moving data).
    Image(int width, int height, int channels, std::vector<uint8_t>&& data);

    /// Copy constructor (deep copy).
    Image(const Image& other);

    /// Copy assignment operator (deep copy).
    Image& operator=(const Image& other);

    /// Move constructor.
    Image(Image&& other) noexcept;

    /// Move assignment operator.
    Image& operator=(Image&& other) noexcept;

    /// Destructor.
    ~Image() = default;

    // --- Dimensions and Queries ---

    /// Returns the width of the image in pixels.
    int width() const noexcept { return width_; }

    /// Returns the height of the image in pixels.
    int height() const noexcept { return height_; }

    /// Returns the number of channels per pixel.
    int channels() const noexcept { return channels_; }

    /// Returns the total number of pixels (width * height).
    size_t totalPixels() const noexcept {
        return static_cast<size_t>(width_) * static_cast<size_t>(height_);
    }

    /// Returns the total number of bytes in the pixel buffer (width * height * channels).
    size_t totalBytes() const noexcept {
        return totalPixels() * static_cast<size_t>(channels_);
    }

    /// Checks if the image has zero dimensions or empty data.
    bool empty() const noexcept {
        return width_ <= 0 || height_ <= 0 || channels_ <= 0 || data_.empty();
    }

    /// Checks if the image dimensions and buffer size are valid and consistent.
    bool isValid() const noexcept {
        return !empty() && (data_.size() == totalBytes());
    }

    // --- Pixel Access (Safe with Bounds Checking) ---

    /**
     * @brief Gets pixel value at (x, y, channel) with bounds checking.
     * @throws std::out_of_range if coordinates or channel are invalid.
     */
    uint8_t getPixel(int x, int y, int channel) const;

    /**
     * @brief Convenience getter for 1-channel (grayscale) images with bounds checking.
     * @throws std::out_of_range if coordinates are invalid.
     */
    uint8_t getPixel(int x, int y) const {
        return getPixel(x, y, 0);
    }

    /**
     * @brief Sets pixel value at (x, y, channel) with bounds checking.
     * @throws std::out_of_range if coordinates or channel are invalid.
     */
    void setPixel(int x, int y, int channel, uint8_t value);

    /**
     * @brief Convenience setter for 1-channel (grayscale) images with bounds checking.
     * @throws std::out_of_range if coordinates are invalid.
     */
    void setPixel(int x, int y, uint8_t value) {
        setPixel(x, y, 0, value);
    }

    // --- Fast Pixel Access (Unchecked for Performance-Critical Loops) ---

    /**
     * @brief Gets pixel value without bounds checking.
     */
    uint8_t getPixelFast(int x, int y, int channel) const noexcept {
        return data_[(static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x))
                     * static_cast<size_t>(channels_) + static_cast<size_t>(channel)];
    }

    /**
     * @brief Convenience fast getter for 1-channel (grayscale) images without bounds checking.
     */
    uint8_t getPixelFast(int x, int y) const noexcept {
        return getPixelFast(x, y, 0);
    }

    /**
     * @brief Sets pixel value without bounds checking.
     */
    void setPixelFast(int x, int y, int channel, uint8_t value) noexcept {
        data_[(static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x))
              * static_cast<size_t>(channels_) + static_cast<size_t>(channel)] = value;
    }

    /**
     * @brief Convenience fast setter for 1-channel (grayscale) images without bounds checking.
     */
    void setPixelFast(int x, int y, uint8_t value) noexcept {
        setPixelFast(x, y, 0, value);
    }

    // --- Raw Buffer Access ---

    /// Returns a const pointer to the raw pixel buffer.
    const uint8_t* data() const noexcept { return data_.data(); }

    /// Returns a mutable pointer to the raw pixel buffer.
    uint8_t* data() noexcept { return data_.data(); }

    /// Returns a const reference to the underlying std::vector.
    const std::vector<uint8_t>& rawData() const noexcept { return data_; }

    /// Returns a mutable reference to the underlying std::vector.
    std::vector<uint8_t>& rawData() noexcept { return data_; }

private:
    int width_{0};
    int height_{0};
    int channels_{0};
    std::vector<uint8_t> data_;

    /// Calculates 1D index from (x, y, channel) with strict bounds checking.
    size_t getIndex(int x, int y, int channel) const;
};

} // namespace fer

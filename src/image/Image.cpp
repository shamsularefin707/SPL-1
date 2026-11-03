#include "Image.h"
#include <sstream>

namespace fer {

Image::Image()
    : width_(0), height_(0), channels_(0), data_() {}

Image::Image(int width, int height, int channels, uint8_t initialValue)
    : width_(width), height_(height), channels_(channels) {
    if (width < 0 || height < 0 || channels < 0) {
        throw std::invalid_argument("Image dimensions and channels must be non-negative.");
    }
    size_t bytes = totalBytes();
    if (bytes > 0) {
        data_.assign(bytes, initialValue);
    }
}

Image::Image(int width, int height, int channels, const std::vector<uint8_t>& data)
    : width_(width), height_(height), channels_(channels), data_(data) {
    if (width < 0 || height < 0 || channels < 0) {
        throw std::invalid_argument("Image dimensions and channels must be non-negative.");
    }
    if (data_.size() != totalBytes()) {
        std::ostringstream oss;
        oss << "Image buffer size mismatch: expected " << totalBytes()
            << " bytes (" << width << "x" << height << "x" << channels
            << "), but got " << data_.size() << " bytes.";
        throw std::invalid_argument(oss.str());
    }
}

Image::Image(int width, int height, int channels, std::vector<uint8_t>&& data)
    : width_(width), height_(height), channels_(channels), data_(std::move(data)) {
    if (width < 0 || height < 0 || channels < 0) {
        throw std::invalid_argument("Image dimensions and channels must be non-negative.");
    }
    if (data_.size() != totalBytes()) {
        std::ostringstream oss;
        oss << "Image buffer size mismatch: expected " << totalBytes()
            << " bytes (" << width << "x" << height << "x" << channels
            << "), but got " << data_.size() << " bytes.";
        throw std::invalid_argument(oss.str());
    }
}

Image::Image(const Image& other)
    : width_(other.width_),
      height_(other.height_),
      channels_(other.channels_),
      data_(other.data_) {}

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        data_ = other.data_;
    }
    return *this;
}

Image::Image(Image&& other) noexcept
    : width_(other.width_),
      height_(other.height_),
      channels_(other.channels_),
      data_(std::move(other.data_)) {
    other.width_ = 0;
    other.height_ = 0;
    other.channels_ = 0;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        data_ = std::move(other.data_);
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
    }
    return *this;
}

size_t Image::getIndex(int x, int y, int channel) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        std::ostringstream oss;
        oss << "Pixel coordinate (" << x << ", " << y << ") out of bounds for image size ("
            << width_ << "x" << height_ << ").";
        throw std::out_of_range(oss.str());
    }
    if (channel < 0 || channel >= channels_) {
        std::ostringstream oss;
        oss << "Channel index " << channel << " out of bounds for image with "
            << channels_ << " channels.";
        throw std::out_of_range(oss.str());
    }
    return (static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x))
           * static_cast<size_t>(channels_) + static_cast<size_t>(channel);
}

uint8_t Image::getPixel(int x, int y, int channel) const {
    size_t idx = getIndex(x, y, channel);
    return data_[idx];
}

void Image::setPixel(int x, int y, int channel, uint8_t value) {
    size_t idx = getIndex(x, y, channel);
    data_[idx] = value;
}

} // namespace fer

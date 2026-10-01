#include <Image.h>
#include <sstream>

namespace fer{
    Image::Image() : width_(0), height_(0), channels_(0), data_() {}

    Image::Image(int width, int height, int channels, uint8_t initialValue):
        width_(width), height_(height), channels_(channels){
            if(width_ <= 0 || height_ <= 0 || channels_ <= 0){
                throw invalid_argument("Image dimensions and channels must be positive.");
            }
            size_t bytes = totalBytes();
            if(bytes > 0){
                data_.assign(bytes, initialValue);
            }
        }
    
    Image::Image(int width, int height, int channels, vector<uint8_t>& data):
        width_(width), height_(height), channels_(channels), data_(data){
            if(width_ <= 0 || height_ <= 0 || channels_ <= 0 || data_.size() != totalBytes()){
                throw invalid_argument("Image dimensions and channels must be positive and data size must match.");
            }
            if(data_.size() != totalBytes()){
                ostringstream ss;
                ss << "Data size does not match image dimensions. Expected: " << totalBytes()
                   << ", but got: " << data_.size();
                throw invalid_argument(ss.str());
            }
        }

    Image::Image(const Image& other):
        width_(other.width_),
        height_(other.height_),
        channels_(other.channels_),
        data_(other.data_){
    }


    Image& Image::operator = (const Image& other){
        if(this != &other){
            width_ = other.width_;
            height_ = other.height_;
            channels_ = other.channels_;
            data_ = other.data_;
        }
        return *this;
    }

    Image::Image(Image&& other) noexcept{
        if(this != &other){
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
        if(x < 0 || x >= width_ || y < 0 || y >= height_ || channel < 0 || channel >= channels_){
            throw out_of_range("Pixel coordinates or channel index out of range.");
        }
        return (static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x)) * 
            static_cast<size_t>(channels_) + static_cast<size_t>(channel);
    }

    uint8_t Image::getPixel(int x, int y, int channel) const{
        size_t index = getIndex(x, y, channel);
        return data_[index];
    }

    void Image::setPixel(int x, int y, int channel, uint8_t value){
        size_t index = getIndex(x, y, channel);
        data_[index] = value;
    }
}

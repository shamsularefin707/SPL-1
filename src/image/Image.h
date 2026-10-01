#pragma once

#include <bits/stdc++.h>
#include <stdexcept>
#include <stdint-gcc.h>
#include <stddef.h>

using namespace std;

namespace fer{
    class Image{
        public:
            Image();

            Image(int width, int height, int channels, uint8_t initialValue = 0);
            
            Image(int width, int height, int channels, vector<uint8_t>& data);

            Image(int width, int height, int channels, vector<uint8_t>&& data);

            Image(const Image& other);

            Image& operator = (const Image& other);

            Image(Image&& other) noexcept;

            Image& operator = (Image&& other) noexcept;

            ~Image() = default;

            //Dimensions and Queries

            //Returns width, height and channels of the image
            int width() const noexcept {return width_;}
            int height() const noexcept {return height_;}
            int channels() const noexcept {return channels_;}

            size_t totalPixels() const noexcept{
                return static_cast<size_t>(width_) * static_cast<size_t>(height_);
            }

            size_t totalBytes() const noexcept{
                return static_cast<size_t>(width_) * static_cast<size_t>(height_) * static_cast<size_t>(channels_);
            }

            bool isEmpty() const noexcept{
                return width_ <= 0 || height_ <= 0 || channels_ <= 0;
            }

            bool isValid() const noexcept{
                return !isEmpty() && data_.size() == totalBytes();
            }

            //Pixel Access
            uint8_t getPixel(int x, int y, int channel) const;

            uint8_t getPixel(int x, int y) const{
                return getPixel(x, y, 0);
            }

            void setPixel(int x, int y, int channel, uint8_t value);
            
            void setPixel(int x, int y, uint8_t value){
                setPixel(x, y, 0, value);
            }

            //Fast Pixel Access

            uint8_t getPixelFast(int x, int y, int channel) const noexcept{
                return data_[(static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x))*
                static_cast<size_t>(channels_) + static_cast<size_t>(channel)];
            }

            uint8_t getPixelFast(int x, int y) const noexcept{
                return getPixelFast(x, y, 0);
            }

            void setPixelFast(int x, int y, int channel, uint8_t value) noexcept{
                data_[(static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x))*
                static_cast<size_t>(channels_) + static_cast<size_t>(channel)] = value;
            }

            void setPixelFast(int x, int y, uint8_t value) noexcept{
                setPixelFast(x, y, 0, value);
            }

            //Data Access
            const uint8_t* data() const noexcept{
                return data_.data();
            }
            
            uint8_t* data() noexcept{
                return data_.data();
            }

            const vector<uint8_t>& dataVector() const noexcept{
                return data_;
            }

            vector<uint8_t>& dataVector() noexcept{
                return data_;
            }

            private:
                int width_;
                int height_;
                int channels_;
                vector<uint8_t> data_;

                size_t getIndex(int x, int y, int channel) const;
    
    };
}
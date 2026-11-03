#pragma once

/**
 * @file ImageProcessing.h
 * @brief Umbrella header for the entire FER Image Processing module.
 *
 * Future modules (Face Detection and Facial Expression Recognition) can include
 * this single header to access all image representation, I/O, and preprocessing
 * facilities, or include individual headers as needed.
 */

#include "Image.h"
#include "ImageLoader.h"
#include "Grayscale.h"
#include "Resize.h"
#include "Contrast.h"
#include "Histogram.h"
#include "Normalization.h"
#include "Rotate.h"
#include "TextRender.h"

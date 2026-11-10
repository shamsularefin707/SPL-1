# FaceMatrix
## Facial expression recognition in C++ with PCA and eigenfaces

FaceMatrix is my Software Project Lab 1 (SPL-1) project at the Institute of Information Technology (IIT), University of Dhaka. It recognises facial expressions in photographs following the method of:

> N. A. Tawhid, N. U. Laskar and H. Ali, *"A Vision-based Facial Expression Recognition and Adaptation System from Video Stream"*, International Journal of Machine Learning and Computing, Vol. 2, No. 5, October 2012. (`FER_Tawhid_Sir.pdf` in this repository)

Everything is written **from scratch in C++17**, using only the C++ standard library. There is no OpenCV, Eigen, stb_image or any other image or maths library.

---

## Project status

The project has three modules. They are being added to this repository one at a time.

| Module | What it does | Status |
|---|---|---|
| **1. Image Processing** | Image data structure, BMP reading/writing, grayscale, resize, contrast stretching, histogram equalisation, rotation, text drawing | ✅ **Uploaded, builds, all tests pass** |
| 2. Face Detection | Image pyramid, sliding-window template matching, eigenspace face verification, non-maximum suppression | ⏳ Coming next |
| 3. Expression Recognition | PCA / eigenfaces, k-nearest-neighbour classification of the expression | ⏳ Planned |

---

## Module 1: Image Processing

Module 1 is the base that the other two modules are built on. It implements Section III-B ("Image Normalization") of the paper:

```text
BMP file ──loadBMP──▶ colour image (W × H × 3)
         ──toGrayscale──────────────▶ W × H × 1       Gray = (R + G + B) / 3        (Eq. 1)
         ──resize (bilinear)────────▶ 132 × 132 × 1                                 (Eq. 2)
         ──percentileContrastStretch▶ 1st–99th percentile → 0–255                   (Eq. 3)
         ──equalizeHistogram────────▶ CDF-based look-up table                       (Eq. 4–5)
         ══▶ normalised 132 × 132 face, ready for PCA
```

`preprocessForFER(image)` runs all four steps in one call.

### Files

| File | Purpose |
|---|---|
| `src/image/Image.h/.cpp` | `fer::Image`: 8-bit pixels stored row-major, any number of channels, checked and fast pixel access, deep copy and move |
| `src/image/ImageLoader.h/.cpp` | Reads and writes uncompressed BMP files (8-, 24- and 32-bit; row padding; bottom-up and top-down rows; BGR ↔ RGB) |
| `src/image/Grayscale.h/.cpp` | RGB → gray using the paper's (R + G + B) / 3 |
| `src/image/Resize.h/.cpp` | Bilinear and nearest-neighbour resizing with centre-aligned pixel mapping |
| `src/image/Contrast.h/.cpp` | Contrast stretching: manual limits, automatic min/max, and percentile (paper Eq. 3), all done with a 256-entry look-up table |
| `src/image/Histogram.h/.cpp` | Histogram, cumulative distribution (CDF) and histogram equalisation |
| `src/image/Normalization.h/.cpp` | `preprocessForFER()`, the complete pipeline above |
| `src/image/Rotate.h/.cpp` | Rotates an image about its centre; samples a tilted rectangle as an upright patch (needed by Module 2 for tilted faces) |
| `src/image/TextRender.h/.cpp` | A 5 × 7 bitmap font for writing labels on result images |
| `src/image/ImageProcessing.h` | Umbrella header: includes all of the headers above |
| `src/main.cpp` | Demo program: runs every step and saves each intermediate image |
| `tests/test_image_processing.cpp` | Unit tests (8 test suites) |
| `Makefile` | Build file |

### Folder layout

```text
.
├── Makefile
├── README.md
├── FER_Tawhid_Sir.pdf               # reference paper
├── src/
│   ├── main.cpp                     # Module 1 demo
│   └── image/                       # Module 1 source code
└── tests/
    └── test_image_processing.cpp    # Module 1 unit tests
```

---

## How to build and run

**Requirements:** a C++17 compiler (g++ 8 or newer, or clang++) and GNU Make.

```bash
make            # compile the demo and the tests
make test       # run the unit tests
make run        # run the demo
make clean      # remove everything the build created
```

**Windows:** install MinGW-w64 (for example through MSYS2), then either use the commands above in Git Bash / MSYS2, or use `mingw32-make`, `mingw32-make test` and `mingw32-make run` in cmd or PowerShell (`mingw32-make clean-win` to clean up).

### Demo

```bash
./fer_image_demo                 # makes a synthetic face picture and processes it
./fer_image_demo my_photo.bmp    # processes your own 24-bit BMP instead
```

The demo saves one image per step in the current folder:

| File | Step |
|---|---|
| `sample_input.bmp` | The input (synthetic face when no file is given) |
| `demo_1_grayscale.bmp` | After grayscale conversion |
| `demo_2_resized_132x132.bmp` | After resizing to 132 × 132 |
| `demo_3_contrast_stretched.bmp` | After contrast stretching |
| `demo_4_histogram_equalized.bmp` | After histogram equalisation |
| `demo_5_pipeline_result.bmp` | `preprocessForFER()` output, all steps in one call |

Only BMP files are supported. Convert PNG or JPEG images to 24-bit BMP first (for example with GIMP or Paint).

### Tests

`make test` runs 8 test suites and stops at the first failure, printing which check failed and on which line:

| Suite | What it checks |
|---|---|
| Image basics | Constructors, sizes, pixel get/set, out-of-range errors, deep copy, move |
| BMP load/save | Save then reload 24-bit (7 × 9) and 8-bit (5 × 7) images; widths chosen so every row needs padding |
| Grayscale | Exact (R + G + B) / 3 values; a gray image stays unchanged |
| Resize | Output size, smooth (monotonic) bilinear values, 200 × 200 → 132 × 132 |
| Contrast stretching | Exact mapped values, low = high step case, low > high error, automatic min/max |
| Percentile stretching | 5 % / 95 % cut-offs, constant image, invalid percentiles, colour image |
| Histogram & equalisation | 256 bins summing to W × H, valid output, constant image (no division by zero) |
| Full pipeline | Any input size → 132 × 132, 1 channel, for both versions of `preprocessForFER` |

Expected output:

```text
[TEST] Image class basics & memory management... PASSED
...
  ALL UNIT TESTS PASSED SUCCESSFULLY! (8/8 suites)
```

---

## Author

**Md. Shamsul Arefin**, BSSE, Institute of Information Technology, University of Dhaka

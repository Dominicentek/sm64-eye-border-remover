#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "portable-file-dialogs.h"

#include <iostream>
#include <cfloat>

#define  PXL(x, y, p) src[(y * height + x) * 4 + p]
#define WPXL(x, y, p) dst[(y * height + x) * 4 + p]

void write_closest_color(unsigned char* src, unsigned char* dst, int x, int y, int width, int height) {
    if (PXL(x, y, 3) != 0x00) return;
    float r = PXL(x, y, 0);
    float g = PXL(x, y, 1);
    float b = PXL(x, y, 2);
    int colors = 1;
    double dist = DBL_MAX;
    for (int X = 0; X < width; X++) {
        for (int Y = 0; Y < height; Y++) {
            if (X == x && Y == y) continue;
            int R = PXL(X, Y, 0);
            int G = PXL(X, Y, 1);
            int B = PXL(X, Y, 2);
            int A = PXL(X, Y, 3);
            if (A == 0) continue;
            double distance = sqrt((X - x) * (X - x) + (Y - y) * (Y - y));
            if (dist == distance) {
                r += R;
                g += G;
                b += B;
                colors++;
            }
            if (dist > distance) {
                r = R;
                g = G;
                b = B;
                colors = 1;
                dist = distance;
            }
        }
    }
    WPXL(x, y, 0) = r / colors;
    WPXL(x, y, 1) = g / colors;
    WPXL(x, y, 2) = b / colors;
    WPXL(x, y, 3) = 0;
}

bool convert_image(char* path) {
    int width, height, channels = 0;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (data && channels == 4) {
        unsigned char* pixels = (unsigned char*)malloc(width * height * 4);
        memcpy(pixels, data, width * height * 4);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                write_closest_color(data, pixels, x, y, width, height);
            }
        }
        stbi_write_png((std::filesystem::path(path).stem().string() + "_converted.png").c_str(), width, height, channels, pixels, width * channels);
        stbi_image_free(data);
        free(pixels);
        std::cout << "...success" << std::endl;
        return false;
    }
    else std::cout << "...failed" << std::endl;
    return true;
}

int main(int argc, char** argv) {
    if (argc == 1) {
#ifdef WINDOWS
        pfd::message("Error", "No images were inputted\nDrag and drop images onto the executable", pfd::choice::ok, pfd::icon::error);
#else
        std::cout << "usage:\neye-border-remover FILE ..." << std::endl;
#endif
        return 1;
    }
    bool failed = false;
    for (int i = 1; i < argc; i++) {
        std::cout << "converting " << argv[i] << std::flush;
        failed |= convert_image(argv[i]);
    }
#ifdef WINDOWS
    if (failed) {
        pfd::message("Error", "One or more images failed to convert", pfd::choice::ok, pfd::icon::error);
    }
    else {
        pfd::message("Success", "All images have been converted", pfd::choice::ok, pfd::icon::error);
    }
#else
    std::cout << "done" << std::endl;
#endif
}

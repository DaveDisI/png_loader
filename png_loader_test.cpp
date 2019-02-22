#include "utilities.h"
#include "os_operations.h"
#include "png_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv){
    u64 fileLength = 0;
    u8* fileData = readFileAsByteArray("grid.png", &fileLength);
    u32 w, h, bpp;
    u8* dats = getPixelDataFromPNGImage(fileData, &w, &h, &bpp);

    freeFileData(&fileData);
    // s32 w, h, bpp;
    // u8* dats = stbi_load("grid.png", &w, &h, &bpp, 0);
    return 0;
}

//0:    255, 0, 0, 255      0, 0, 255, 255      0, 255, 0, 255, 
//0:    255, 255, 0, 255    0, 0, 0, 0          255, 0, 255, 255
//2:    0, 0, 255, 0           0 0 0 255       1, 255, 0, 0
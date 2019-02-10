#include "utilities.h"
#include "os_operations.h"
#include "png_loader.h"

int main(int argc, char** argv){
    u64 fileLength = 0;
    u8* fileData = readFileAsByteArray("bfly.png", &fileLength);
    u32 w, h;
    u8 bpp;
    u8* imgPixelData = getPixelDataFromPNGImage(fileData, &w, &h, &bpp);
    freeFileData(&fileData);
    return 0;
}
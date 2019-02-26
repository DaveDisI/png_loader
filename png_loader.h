#pragma once

#include "utilities.h"

#define PNG_HEADER 727905341920923785

struct PNGHuffman {
    u32 totalCodes;
    u32 minBitLength;
    u32 maxBitLength;
    u32* codes;
    u32* values;
};

u32 readBitsFromArray(u8* arr, u32 offset, u8 numBits){
    u32 res = 0;

    u8 byteOffset = offset / 8;
    u8 bitOffset = offset % 8;

    for(u32 i = 0; i < numBits; i++){
        u8 cByte = arr[byteOffset];
        u32 r = (cByte & (1 << bitOffset)) >> bitOffset;
        res |= r << i;
        bitOffset++;
        if(bitOffset % 8 == 0){
            bitOffset = 0;
            byteOffset++;
        }
    }

    return res;
}

u32 readBitsFromArrayReversed(u8* arr, u32 offset, u8 numBits){
    u32 res = 0;

    u8 byteOffset = offset / 8;
    u8 bitOffset = offset % 8;

    for(u32 i = 0; i < numBits; i++){
        u8 cByte = arr[byteOffset];
        u32 r = (cByte & (1 << bitOffset)) >> bitOffset;
        res |= r << (numBits - i - 1);
        bitOffset++;
        if(bitOffset % 8 == 0){
            bitOffset = 0;
            byteOffset++;
        }
    }

    return res;
}

PNGHuffman generatePNGHuffmanFromCodeLengths(u32 totalCodes, u32* lengths, u32 maxBits){
    u32* b1Count = ALLOCATE_MEMORY(u32, maxBits + 1);
    u32* nextCode = ALLOCATE_MEMORY(u32, maxBits + 1);
    memset(b1Count, 0, (maxBits + 1) * sizeof(u32));
    memset(nextCode, 0, (maxBits + 1) * sizeof(u32));
    for(u32 i = 0; i < totalCodes; i++){
        b1Count[lengths[i]]++;
    }

    u32 code = 0;
    b1Count[0] = 0;
    for(u32 i = 1; i <= maxBits; i++){
        code = (code + b1Count[i - 1]) << 1;
        nextCode[i] = code;
    }  

    u32* codes = ALLOCATE_MEMORY(u32, totalCodes);
    u32* values = ALLOCATE_MEMORY(u32, totalCodes);
    u32 totalCodesUsed = 0;
    memset(codes, 0, totalCodes * sizeof(u32));
    memset(values, 0, totalCodes * sizeof(u32));

    u32 minLen = -1;
    u32 maxLen = 0;

    for(u32 i = 0; i < totalCodes; i++){
        u32 len = lengths[i];
        if(len != 0){
            if(len < minLen) minLen = len;
            if(len > maxLen) maxLen = len;
            codes[totalCodesUsed] = nextCode[len];
            values[totalCodesUsed] = i;
            totalCodesUsed++;
            nextCode[len]++;
        }
    } 

    free(b1Count);
    free(nextCode);

    for(u32 i = 0; i < totalCodesUsed - 1; i++){
        for(u32 j =  i + 1; j < totalCodesUsed; j++){
            if(codes[i] > codes[j]){
                u32 t = codes[i];
                codes[i] = codes[j];
                codes[j] = t;
                t = values[i];
                values[i] = values[j];
                values[j] = t;
            }
        }
    }

    PNGHuffman pngh;
    pngh.minBitLength = minLen;
    pngh.maxBitLength = maxLen;
    pngh.totalCodes = totalCodesUsed;
    pngh.codes = codes;
    pngh.values = values;
    return pngh;
}

u32 parseHuffmanCodeFromData(u8* data, u32* offset, PNGHuffman* pngh){
    for(u32 i = pngh->minBitLength; i <= pngh->maxBitLength; i++){
        u32 hufCode = readBitsFromArrayReversed(data, *offset, i);
        for(u32 j = 0; j < pngh->totalCodes; j++){
            if(hufCode == pngh->codes[j]){
                *offset += i;
                return pngh->values[j];
            }else if(pngh->codes[j] > hufCode){
                break;
            }
        }
    }
    return -1;
}

u8* getPixelDataFromPNGImage(u8* fileData, u32* width, u32* height, u32* bitsPerPixel){
    u64 pngHeader = *(u64*)fileData;
    fileData += 8;
    if(pngHeader != PNG_HEADER){
        printf("%llu\n", pngHeader);
        return 0;
    }

    u8* compressedData = 0;
    u32 totalCompressedDataSize = 0;

    u32 totalImageSize = 0;

    bool continueThrougFile = true;
    while(continueThrougFile){
        u32 chunkLength = SWAP32(*(u32*)fileData);
        fileData += 4;
        u32 chunkType = SWAP32(*(u32*)fileData);
        fileData += 4;
        if(STR_TO_INT("IHDR") == chunkType){
            *width = SWAP32(*(u32*)fileData);
            fileData += 4;
            *height = SWAP32(*(u32*)fileData);
            fileData += 4;
            u8 bitDepth = *fileData++;
            u8 colorType = *fileData++;
            u8 compressionMethod = *fileData++;
            u8 filterMethod = *fileData++;
            u8 interlaceMethod = *fileData++;

            switch(colorType){
                case 0:
                case 3:{
                    *bitsPerPixel = bitDepth;
                    break;
                }
                case 2:{
                    *bitsPerPixel = bitDepth * 3;
                    break;
                }
                case 4:{
                    *bitsPerPixel = bitDepth * 3;
                    break;
                }
                case 6:{
                    *bitsPerPixel = bitDepth * 4;
                    break;
                }
            }

            totalImageSize = *width * *height * ((*bitsPerPixel / 8) > 0 ? (*bitsPerPixel / 8) : 1);
            printf("W:%u\tH:%u\n", *width, *height); 
            printf("tis: %u\n", totalImageSize); 
        }else if(STR_TO_INT("IDAT") == chunkType){
            totalCompressedDataSize += chunkLength;
            compressedData = (u8*)realloc(compressedData, totalCompressedDataSize);
            for(u32 i = totalCompressedDataSize - chunkLength; i < totalCompressedDataSize; i++){
                compressedData[i] = *fileData++;    
            }
        }else if(STR_TO_INT("IEND") == chunkType){ 
            continueThrougFile = false;
        }else{
            fileData += chunkLength;
        }
        u32 crc = SWAP32(*(u32*)fileData);
        fileData += 4;
    }

    u8 cmf = *compressedData++;
    u8 flg = *compressedData++;
    u8 cm = readBitsFromArray(&cmf, 0, 4);
    u8 cinfo = readBitsFromArray(&cmf, 4, 4);
    u8 fcheck = readBitsFromArray(&flg, 0, 5);
    u8 fdict = readBitsFromArray(&flg, 5, 1);
    u8 flevel = readBitsFromArray(&flg, 6, 2);
    if(fdict) { compressedData += 4; }

    u32 offset = 0;
    u8 bfinal = readBitsFromArray(compressedData, offset++, 1);
    u8 btype = readBitsFromArray(compressedData, offset, 2);
    offset += 2;
    
    printf("bfinal: %u\n", bfinal);
    printf("btype: %u\n", btype);
    
    switch(btype){
        case 3: {
            return 0;
        }
        case 0: {
            break;
        }
        case 1: {
            break;
        }
        case 2: {
            u32 hlit = readBitsFromArray(compressedData, offset, 5) + 257;
            offset += 5;
            u32 hdist = readBitsFromArray(compressedData, offset, 5) + 1;
            offset += 5;
            u32 hclen = readBitsFromArray(compressedData, offset, 4) + 4;
            offset += 4;

            printf("hclen: \t%u\n", hclen);
            printf("hlit: \t%u\n", hlit);
            printf("hdist: \t%u\n", hdist);

            u8 codeLengthAlphabet[] = {
                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
            }; 

            u32 codeLengths[19] = {};

            for(u32 i = 0; i < hclen; i++){
                codeLengths[codeLengthAlphabet[i]] = readBitsFromArray(compressedData, offset, 3);
                offset += 3;
            }

            PNGHuffman cLenHuff = generatePNGHuffmanFromCodeLengths(19, codeLengths, 7);

            u32 lenDistTotal = hlit + hdist;
            u32 lenDistFound = 0;
            u32* lenDistCodeLengths = ALLOCATE_MEMORY(u32, lenDistTotal);

            while(lenDistFound < lenDistTotal){
                u32 code = parseHuffmanCodeFromData(compressedData, &offset, &cLenHuff);
                if(code < 16){
                    lenDistCodeLengths[lenDistFound++] = code;
                }else if(code == 16){
                    u32 extraBits = readBitsFromArray(compressedData, offset, 2) + 3;
                    offset += 2;
                    for(u32 i = 0; i < extraBits; i++){
                        lenDistCodeLengths[lenDistFound++] = lenDistCodeLengths[lenDistFound - 1];
                    }
                }else if(code == 17){
                    u32 extraBits = readBitsFromArray(compressedData, offset, 3) + 3;
                    offset += 3;
                    for(u32 i = 0; i < extraBits; i++){
                        lenDistCodeLengths[lenDistFound++] = 0;
                    }
                }else if(code == 18){
                    u32 extraBits = readBitsFromArray(compressedData, offset, 7) + 11;
                    offset += 7;
                    for(u32 i = 0; i < extraBits; i++){
                        lenDistCodeLengths[lenDistFound++] = 0;
                    }
                }else{
                    return 0;
                }
            }

            PNGHuffman litLenHuff = generatePNGHuffmanFromCodeLengths(hlit, lenDistCodeLengths, 15);
            PNGHuffman distHuff = generatePNGHuffmanFromCodeLengths(hdist, lenDistCodeLengths + hlit, 15);

            while(true){
                u32 code = parseHuffmanCodeFromData(compressedData, &offset, &litLenHuff);
                if(code == 256){
                    printf("256 END!\n");
                    break;
                }else if(code < 256){
                    printf("code: %u\n", code);
                }else if(code < 265){
                    u32 distCode = parseHuffmanCodeFromData(compressedData, &offset, &distHuff);
                    u32 length = code - 254;
                    printf("code: %u\tlen: %u\tdist:%u\n", code, length, distCode);
                }else if(code < 268){
                
                }
            }
            
            break;
        }
        default : {}
    }

    return 0;
}

void freeImageData(u8** imageData){
    if(*imageData){
        delete[] *imageData;
        *imageData = 0;
    }
}
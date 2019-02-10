#pragma once

#include "utilities.h"

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

u32 readBitsFromArrayRvs(u8* arr, u32 offset, u8 numBits){
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

u8* getPixelDataFromPNGImage(u8* fileData, u32* width, u32* height, u8* bitsPerPixel){
    const u64 PNG_HEADER = 1196314761;
    u64 pngHeader = *(u64*)fileData;
    fileData += 8;
    if(pngHeader != PNG_HEADER){
        return 0;
    }

    u8* compressedData = 0;
    u32 totalCompressedDataSize = 0;

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

            u8 codeLengthAlphabet[] = {
                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
            }; 

            u8 codeLengths[19];

            for(u32 i = 0; i < hclen; i++){
                codeLengths[i] = readBitsFromArray(compressedData, offset, 3);
                offset += 3;
            }

            u32 smallestIndex = 0;
            u32 sortStart = 0;
            bool sorted = false;
            while(!sorted){
                for(u32 i = sortStart + 1; i < hclen; i++){
                    if(codeLengths[i] < codeLengths[smallestIndex]){
                        smallestIndex = i;
                    }
                }

                for(u32 i = smallestIndex; i > sortStart; i--){
                    u32 t = codeLengths[i];
                    codeLengths[i] = codeLengths[i - 1];
                    codeLengths[i - 1] = t;
                    t = codeLengthAlphabet[i];
                    codeLengthAlphabet[i] = codeLengthAlphabet[i - 1];
                    codeLengthAlphabet[i - 1] = t;
                }
                sortStart++;

                for(u32 i = sortStart; i < hclen; i++){
                    if(codeLengths[i] == codeLengths[sortStart - 1]){
                        for(u32 j = i; j > sortStart; j--){
                            u32 t = codeLengths[j];
                            codeLengths[j] = codeLengths[j - 1];
                            codeLengths[j - 1] = t;
                            t = codeLengthAlphabet[j];
                            codeLengthAlphabet[j] = codeLengthAlphabet[j - 1];
                            codeLengthAlphabet[j - 1] = t;
                        }
                        sortStart++;
                    }
                }

                smallestIndex = sortStart;

                sorted = true;
                for(u32 i = 0; i < hclen - 1; i++){
                    if(codeLengths[i] > codeLengths[i + 1]){
                        sorted = false;
                    }
                }
            }

            u8 codes[19] = {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            };

            u8 bitLengths[7];
            u8 totalUsedBitLengths = 0;

            u8 code = 0;
            u8 count = 1;
            u8 lastCode = 0;
            u8 blStartIndex = 0;
            for(u32 i = 1; i < hclen; i++){
                if(codeLengths[i] == codeLengths[i - 1]){
                    count++;
                    if(codeLengths[i] > 0){
                        codes[i] = codes[i - 1] + 1;
                    }else{
                         codes[i] = 0;
                    }
                }else{
                    if(codeLengths[i] != 0){
                        if(totalUsedBitLengths == 0){
                            blStartIndex = i;
                        }
                        bitLengths[totalUsedBitLengths++] = codeLengths[i];
                    }

                    if(codeLengths[i - 1] == 0){
                        codes[i] = 0;
                    }else{
                        codes[i] = (lastCode + count) << 1; 
                        lastCode = codes[i];
                    }
                    count = 1;
                }
            }

            for(u32 i = 0; i < hclen; i++){
                 printf("%u:\t%u:\t%u\n", codeLengthAlphabet[i], codeLengths[i], codes[i]);
            }

            printf("hlit: %u\n", hlit);
            printf("hdist: %u\n", hdist);
            printf("hclen: %u\n", hclen);

            u32 hlitCodeLengths[286];
            u32 hlitCLFound = 0;

            for(u32 i = 2; i < 8; i++){
                u32 cd = readBitsFromArrayRvs(compressedData, offset, i);
                for(u32 j = blStartIndex; j < hclen; j++){
                    if(cd == codes[j]){
                        u32 literalCode = codeLengthAlphabet[j];
                        offset += i;
                        i = 1;
                        switch(literalCode){
                            case 16: {
                                u32 extraBits = readBitsFromArray(compressedData, offset, 2);
                                offset += 2;
                                for(u32 k = 0; k < extraBits; k++){
                                     hlitCodeLengths[hlitCLFound] =  hlitCodeLengths[hlitCLFound - 1];
                                     hlitCLFound++;
                                }
                                break;
                            }
                            case 17: {
                                u32 extraBits = readBitsFromArray(compressedData, offset, 3);
                                offset += 3;
                                for(u32 k = 0; k < extraBits; k++){
                                    hlitCodeLengths[hlitCLFound++] = 0;
                                }
                                break;
                            }
                            case 18: {
                                u32 extraBits = readBitsFromArray(compressedData, offset, 7);
                                offset += 7;
                                for(u32 k = 0; k < extraBits; k++){
                                    hlitCodeLengths[hlitCLFound++] = 0;
                                }
                                break;
                            }
                            default: {
                                hlitCodeLengths[hlitCLFound++] = literalCode;
                                break;
                            }
                        }

                        break;
                    }
                }
                if(hlitCLFound == hlit){
                    break;
                }
            }

            for(u32 i = 0; i < hlit; i++){
                printf("%u:\t%u\n", i, hlitCodeLengths[i]);
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
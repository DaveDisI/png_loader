#pragma once

#include "utilities.h"

#define PNG_HEADER 727905341920923785

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

u8* getPixelDataFromPNGImage(u8* fileData, u32* width, u32* height, u32* bitsPerPixel){
    u64 pngHeader = *(u64*)fileData;
    fileData += 8;
    if(pngHeader != PNG_HEADER){
        printf("%lu\n", pngHeader);
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
            printf("W:%uH:%U\n", *width, *height); 
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

            u8 codeLengths[19];

            for(u32 i = 0; i < hclen; i++){
                codeLengths[i] = readBitsFromArray(compressedData, offset, 3);
                offset += 3;
            }

            for(u32 i = 0; i < hclen - 1; i++){
                for(u32 j = i + 1; j < hclen; j++){
                    if(codeLengthAlphabet[i] > codeLengthAlphabet[j]){
                        u8 t = codeLengthAlphabet[i];
                        codeLengthAlphabet[i] = codeLengthAlphabet[j];
                        codeLengthAlphabet[j] = t;
                        t = codeLengths[i];
                        codeLengths[i] = codeLengths[j];
                        codeLengths[j] = t; 
                    }
                }
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

            u8 codes[19] = {};
            u8 bitLengths[16];
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
                        if(codeLengths[i - 1] != codeLengths[i] - 1){
                            lastCode = (lastCode + count) << 1;
                            codes[i] = lastCode << 1; 
                        }else{
                            codes[i] = (lastCode + count) << 1; 
                        }
                        lastCode = codes[i];
                    }
                    count = 1;
                }
            }

            u32 hlitCodeLengths[318];
            u32 hlitCLFound = 0;

            for(u32 i = 0; i < totalUsedBitLengths; i++){
                u32 cd = readBitsFromArrayReversed(compressedData, offset, bitLengths[i]);
                for(u32 j = blStartIndex; j < hclen; j++){
                    if(cd == codes[j]){
                        u32 literalCode = codeLengthAlphabet[j];
                        offset += bitLengths[i];
                        i = -1;
                        switch(literalCode){
                            case 16: {
                                u32 extraBits = readBitsFromArray(compressedData, offset, 2) + 3;
                                offset += 2;
                                for(u32 k = 0; k < extraBits; k++){
                                     hlitCodeLengths[hlitCLFound] =  hlitCodeLengths[hlitCLFound - 1];
                                     hlitCLFound++;
                                }
                                break;
                            }
                            case 17: {
                                u32 extraBits = readBitsFromArray(compressedData, offset, 3) + 3;
                                offset += 3;
                                for(u32 k = 0; k < extraBits; k++){
                                    hlitCodeLengths[hlitCLFound++] = 0;
                                }
                                break;
                            }
                            case 18: {
                                u32 extraBits = readBitsFromArray(compressedData, offset, 7) + 11;
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
                if(hlitCLFound == hlit + hdist){
                    break;
                }
            }

            u32 alPositions[286] = {};
            u32 actualLengths[286] = {};
            u32 actualLengthCodes[286] = {};
            u32 adPositions[32] = {};
            u32 actualDistances[32] = {};
            u32 actualDistanceCodes[32] = {};
            for(u32 i = 0; i < 286; i++){ alPositions[i] = i; }
            for(u32 i = 0; i < 32; i++){ adPositions[i] = i; }
            copyMemory(hlitCodeLengths, actualLengths, hlit * 4);
            copyMemory(hlitCodeLengths + hlit, actualDistances, hdist * 4);
            

            smallestIndex = 0;
            sortStart = 0;
            sorted = false;
            while(!sorted){
                for(u32 i = sortStart + 1; i < hlit; i++){
                    if(actualLengths[i] < actualLengths[smallestIndex]){
                        smallestIndex = i;
                    }
                }

                for(u32 i = smallestIndex; i > sortStart; i--){
                    u32 t = actualLengths[i];
                    actualLengths[i] = actualLengths[i - 1];
                    actualLengths[i - 1] = t;
                    t = alPositions[i];
                    alPositions[i] = alPositions[i - 1];
                    alPositions[i - 1] = t;
                }
                sortStart++;

                for(u32 i = sortStart; i < hlit; i++){
                    if(actualLengths[i] == actualLengths[sortStart - 1]){
                        for(u32 j = i; j > sortStart; j--){
                            u32 t = actualLengths[j];
                            actualLengths[j] = actualLengths[j - 1];
                            actualLengths[j - 1] = t;
                            t = alPositions[j];
                            alPositions[j] = alPositions[j - 1];
                            alPositions[j - 1] = t;
                        }
                        sortStart++;
                    }
                }

                smallestIndex = sortStart;

                sorted = true;
                for(u32 i = 0; i < hlit - 1; i++){
                    if(actualLengths[i] > actualLengths[i + 1]){
                        sorted = false;
                    }
                }
            }

            smallestIndex = 0;
            sortStart = 0;
            sorted = false;
            while(!sorted){
                for(u32 i = sortStart + 1; i < hdist; i++){
                    if(actualDistances[i] < actualDistances[smallestIndex]){
                        smallestIndex = i;
                    }
                }

                for(u32 i = smallestIndex; i > sortStart; i--){
                    u32 t = actualDistances[i];
                    actualDistances[i] = actualDistances[i - 1];
                    actualDistances[i - 1] = t;
                    t = adPositions[i];
                    adPositions[i] = adPositions[i - 1];
                    adPositions[i - 1] = t;
                }
                sortStart++;

                for(u32 i = sortStart; i < hdist; i++){
                    if(actualDistances[i] == actualDistances[sortStart - 1]){
                        for(u32 j = i; j > sortStart; j--){
                            u32 t = actualDistances[j];
                            actualDistances[j] = actualDistances[j - 1];
                            actualDistances[j - 1] = t;
                            t = adPositions[j];
                            adPositions[j] = adPositions[j - 1];
                            adPositions[j - 1] = t;
                        }
                        sortStart++;
                    }
                }

                smallestIndex = sortStart;

                sorted = true;
                for(u32 i = 0; i < hdist - 1; i++){
                    if(actualDistances[i] > actualDistances[i + 1]){
                        sorted = false;
                    }
                }
            }

            u8 clBitLengths[16] = {};
            u32 totalUsedLCBitLengths = 0;
            count = 1;
            lastCode = 0;
            u32 lcStartIndex = 0;
            for(u32 i = 1; i < hlit; i++){
                if(actualLengths[i] == actualLengths[i - 1]){
                    count++;
                    if(actualLengths[i] > 0){
                        actualLengthCodes[i] = actualLengthCodes[i - 1] + 1;
                    }else{
                        actualLengthCodes[i] = 0;
                    }
                }else{
                    if(actualLengths[i] != 0){
                        if(totalUsedLCBitLengths == 0){
                            lcStartIndex = i;
                        }
                        clBitLengths[totalUsedLCBitLengths++] = actualLengths[i];
                    }

                    if(actualLengths[i - 1] == 0){
                        actualLengthCodes[i] = 0;
                    }else{
                        if(actualLengths[i - 1] != actualLengths[i] - 1){
                            lastCode = (lastCode + count) << 1;
                            actualLengthCodes[i] = lastCode << 1; 
                        }else{
                            actualLengthCodes[i] = (lastCode + count) << 1; 
                        }
                        lastCode = actualLengthCodes[i];
                    }
                    count = 1;
                }
            }

            u8 ldBitLengths[16] = {};
            u32 totalUsedLDBitLengths = 0;
            count = 1;
            lastCode = 0;
            u32 dlStartIndex = 0;
            for(u32 i = 1; i < hdist; i++){
                if(actualDistances[i] == actualDistances[i - 1]){
                    count++;
                    if(actualDistances[i] > 0){
                        actualDistanceCodes[i] = actualDistanceCodes[i - 1] + 1;
                    }else{
                         actualDistanceCodes[i] = 0;
                    }
                }else{
                    if(actualDistances[i] != 0){
                        if(totalUsedLDBitLengths == 0){
                            dlStartIndex = i;
                        }
                        ldBitLengths[totalUsedLDBitLengths++] = actualDistances[i];
                    }

                    if(actualDistances[i - 1] == 0){
                        actualDistanceCodes[i] = 0;
                    }else{
                        if(actualDistances[i - 1] != actualDistances[i] - 1){
                            lastCode = (lastCode + count) << 1;
                            actualDistanceCodes[i] = lastCode << 1; 
                        }else{
                            actualDistanceCodes[i] = (lastCode + count) << 1; 
                        }
                        lastCode = actualDistanceCodes[i];
                    }
                    count = 1;
                }
            }
            
            for(u32 i = 0; i < totalUsedLCBitLengths; i++){
                u32 cd = readBitsFromArrayReversed(compressedData, offset, clBitLengths[i]);
                for(u32 j = lcStartIndex; j < hlit; j++){
                    if(cd == actualLengthCodes[j]){
                        u32 literalCode = alPositions[j];
                        offset += clBitLengths[i];
                        i = -1;
                        if(literalCode == 256){
                            printf("done!\n");
                            i = totalUsedLCBitLengths;
                            break;
                        }else if(literalCode > 256){
                            u32 length = 0;
                            switch(literalCode){
                                case 265:{
                                    length = literalCode - 254;
                                    length += readBitsFromArray(compressedData, offset++, 1);
                                    break;
                                }
                                case 266:{
                                    length = literalCode - 253;
                                    length += readBitsFromArray(compressedData, offset++, 1);
                                    break;
                                }case 267:{
                                    length = literalCode - 252;
                                    length += readBitsFromArray(compressedData, offset++, 1);
                                    break;
                                }case 268:{
                                    length = literalCode - 251;
                                    length += readBitsFromArray(compressedData, offset++, 1);
                                    break;
                                }case 269:{
                                    length = literalCode - 250;
                                    length += readBitsFromArray(compressedData, offset, 2);
                                    offset += 2;
                                    break;
                                }case 270:{
                                    length = literalCode - 247;
                                    length += readBitsFromArray(compressedData, offset, 2);
                                    offset += 2;
                                    break;
                                }case 271:{
                                    length = literalCode - 244;
                                    length += readBitsFromArray(compressedData, offset, 2);
                                    offset += 2;
                                    break;
                                }case 272:{
                                    length = literalCode - 241;
                                    length += readBitsFromArray(compressedData, offset, 2);
                                    offset += 2;
                                    break;
                                }case 273:{
                                    length = literalCode - 238;
                                    length += readBitsFromArray(compressedData, offset, 3);
                                    offset += 3;
                                    break;
                                }case 274:{
                                    length = literalCode - 231;
                                    length += readBitsFromArray(compressedData, offset, 3);
                                    offset += 3;
                                    break;
                                }case 275:{
                                    length = literalCode - 224;
                                    length += readBitsFromArray(compressedData, offset, 3);
                                    offset += 3;
                                    break;
                                }case 276:{
                                    length = literalCode - 217;
                                    length += readBitsFromArray(compressedData, offset, 3);
                                    offset += 3;
                                    break;
                                }case 277:{
                                    length = literalCode - 210;
                                    length += readBitsFromArray(compressedData, offset, 4);
                                    offset += 4;
                                    break;
                                }case 278:{
                                    length = literalCode - 195;
                                    length += readBitsFromArray(compressedData, offset, 4);
                                    offset += 4;
                                    break;
                                } case 279:{
                                    length = literalCode - 180;
                                    length += readBitsFromArray(compressedData, offset, 4);
                                    offset += 4;
                                    break;
                                }case 280:{
                                    length = literalCode - 165;
                                    length += readBitsFromArray(compressedData, offset, 4);
                                    offset += 4;
                                    break;
                                }case 281:{
                                    length = literalCode - 150;
                                    length += readBitsFromArray(compressedData, offset, 5);
                                    offset += 5;
                                    break;
                                }case 282:{
                                    length = literalCode - 119;
                                    length += readBitsFromArray(compressedData, offset, 5);
                                    offset += 5;
                                    break;
                                }case 283:{
                                    length = literalCode - 88;
                                    length += readBitsFromArray(compressedData, offset, 5);
                                    offset += 5;
                                    break;
                                }case 284:{
                                    length = literalCode - 57;
                                    length += readBitsFromArray(compressedData, offset, 5);
                                    offset += 5;
                                    break;
                                }case 285:{
                                    length = 258;
                                    break;
                                }default: {
                                    length = literalCode - 254;
                                    break;
                                }
                            }
                            
                            for(u32 k = 0; k < totalUsedLDBitLengths; k++){
                                u32 dcd = readBitsFromArrayReversed(compressedData, offset, ldBitLengths[k]);
                                for(u32 l = dlStartIndex; l < hdist; l++){
                                    if(dcd == actualDistanceCodes[l]){
                                        u32 distanceCode = adPositions[l];
                                        offset += ldBitLengths[k];
                                        u32 dist = 0;
                                        
                                        if(distanceCode < 4){
                                            dist = distanceCode + 1;
                                        }else if(distanceCode == 4){
                                            dist = 5;
                                            dist += readBitsFromArray(compressedData, offset++, 1);
                                        }else if(distanceCode == 5){
                                            dist = 7;
                                            dist += readBitsFromArray(compressedData, offset++, 1);
                                        }else if(distanceCode == 6){
                                            dist = 9;
                                            dist += readBitsFromArray(compressedData, offset, 2);
                                            offset += 2;
                                        }else if(distanceCode == 7){
                                            dist = 13;
                                            dist += readBitsFromArray(compressedData, offset, 2);
                                            offset += 2;
                                        }else if(distanceCode == 8){
                                            dist = 17;
                                            dist += readBitsFromArray(compressedData, offset, 3);
                                            offset += 3;
                                        }else if(distanceCode == 9){
                                            dist = 25;
                                            dist += readBitsFromArray(compressedData, offset, 3);
                                            offset += 3;
                                        }else if(distanceCode == 10){
                                            dist = 33;
                                            dist += readBitsFromArray(compressedData, offset, 4);
                                            offset += 4;
                                        }else if(distanceCode == 11){
                                            dist = 49;
                                            dist += readBitsFromArray(compressedData, offset, 4);
                                            offset += 4;
                                        }else if(distanceCode == 12){
                                            dist = 65;
                                            dist += readBitsFromArray(compressedData, offset, 5);
                                            offset += 5;
                                        }else if(distanceCode == 13){
                                            dist = 97;
                                            dist += readBitsFromArray(compressedData, offset, 5);
                                            offset += 5;
                                        }else if(distanceCode == 14){
                                            dist = 129;
                                            dist += readBitsFromArray(compressedData, offset, 6);
                                            offset += 6;
                                        }else if(distanceCode == 15){
                                            dist = 193;
                                            dist += readBitsFromArray(compressedData, offset, 6);
                                            offset += 6;
                                        }else if(distanceCode == 16){
                                            dist = 257;
                                            dist += readBitsFromArray(compressedData, offset, 7);
                                            offset += 7;
                                        }else if(distanceCode == 17){
                                            dist = 385;
                                            dist += readBitsFromArray(compressedData, offset, 7);
                                            offset += 7;
                                        }else if(distanceCode == 18){
                                            dist = 513;
                                            dist += readBitsFromArray(compressedData, offset, 8);
                                            offset += 8;
                                        }else if(distanceCode == 19){
                                            dist = 769;
                                            dist += readBitsFromArray(compressedData, offset, 8);
                                            offset += 8;
                                        }else if(distanceCode == 20){
                                            dist = 1025;
                                            dist += readBitsFromArray(compressedData, offset, 9);
                                            offset += 9;
                                        }else if(distanceCode == 21){
                                            dist = 1537;
                                            dist += readBitsFromArray(compressedData, offset, 9);
                                            offset += 9;
                                        }else if(distanceCode == 22){
                                            dist = 2049;
                                            dist += readBitsFromArray(compressedData, offset, 10);
                                            offset += 10;
                                        }else if(distanceCode == 23){
                                            dist = 3073;
                                            dist += readBitsFromArray(compressedData, offset, 10);
                                            offset += 10;
                                        }else if(distanceCode == 24){
                                            dist = 4097;
                                            dist += readBitsFromArray(compressedData, offset, 11);
                                            offset += 11;
                                        }else if(distanceCode == 25){
                                            dist = 6145;
                                            dist += readBitsFromArray(compressedData, offset, 11);
                                            offset += 11;
                                        }else if(distanceCode == 26){
                                            dist = 8193;
                                            dist += readBitsFromArray(compressedData, offset, 12);
                                            offset += 12;
                                        }else if(distanceCode == 27){
                                            dist = 12289;
                                            dist += readBitsFromArray(compressedData, offset, 12);
                                            offset += 12;
                                        }else if(distanceCode == 28){
                                            dist = 16385;
                                            dist += readBitsFromArray(compressedData, offset, 13);
                                            offset += 13;
                                        }else{
                                            dist = 24577;
                                            dist += readBitsFromArray(compressedData, offset, 13);
                                            offset += 13;
                                        }  

                                        printf("%u:%u:%u\t", literalCode, length, dist);
                                    }
                                }
                            }
                        }else {
                            printf("%u, \t", literalCode);
                        }
                        break;
                    }
                }
            }

            for(u32 i = 0; i < hlit; i++){
                printf("%u:\t%u\t%u\t%u\n", i, alPositions[i], actualLengths[i], actualLengthCodes[i]);
            }
            for(u32 i = 0; i < hdist; i++){
                printf("%u:\t%u\t%u\t%u\n", i, adPositions[i], actualDistances[i], actualDistanceCodes[i]);
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
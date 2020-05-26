﻿// Copyright 2006-2010 Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/SilverLiningTypes.h>
#include <FeSilverliningLib/TGALoader.h>
#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/ResourceLoader.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace SilverLining;

typedef struct {
    unsigned char ImgIdent;
    unsigned char ignored[ 1 ];
    unsigned char ImgType;
    unsigned char ignored2[ 9 ];
    unsigned char WidthLo;
    unsigned char WidthHi;
    unsigned char HeightLo;
    unsigned char HeightHi;
    unsigned char Bpp;
    unsigned char ignored3[ 1 ];
} TGAHeader;

TGALoader::TGALoader() : pixels(0)
{
}

TGALoader::~TGALoader()
{
    if (pixels) {
        SL_FREE(pixels);
    }
}

bool TGALoader::Load(const char *filename)
{
    if (pixels) {
        SL_FREE(pixels);
        pixels = 0;
    }

    char *data;
    unsigned int dataLen;
    if (Atmosphere::GetResourceLoader()->LoadResource(filename, data, dataLen, false)) {
        char *p = data;
        TGAHeader header = *((TGAHeader *)(p));
        p += sizeof(TGAHeader);

        // Precalc some values from the header
        const unsigned int imageType  = header.ImgType;
        const unsigned int imageWidth  = header.WidthLo  + header.WidthHi  * 256;
        const unsigned int imageHeight  = header.HeightLo + header.HeightHi * 256;
        const unsigned int imageBytesPerPel = header.Bpp / 8;
        const unsigned int imageSize  = imageWidth * imageHeight * imageBytesPerPel;

        width = imageWidth;
        height = imageHeight;
        bpp = header.Bpp;

        // Validate header info; must be uncompressed and not mapped
        if ( ( imageType != 2 && imageType != 3 && imageType != 10 ) ||
                ( imageWidth == 0 ) || ( imageHeight == 0 ) ||
                ( imageBytesPerPel != 3 && imageBytesPerPel != 4 && imageBytesPerPel != 1) ) {
            // invalid header, bomb out
            Atmosphere::GetResourceLoader()->FreeResource(data);
            return false;
        }

        // Allocate the memory for the image size
        pixels = (unsigned char *)SL_MALLOC(imageSize);

        // Skip image ident field
        if ( header.ImgIdent > 0 ) {
            p += header.ImgIdent;
        }

        // If uncompressed...
        if (imageType == 2 || imageType == 3) {
            memcpy(pixels, p, imageSize);

            if (imageType == 2) {
                for (unsigned int cswap = 0; cswap < imageSize; cswap += imageBytesPerPel) {
                    pixels[cswap] ^= pixels[cswap+2];
                    pixels[cswap+2] ^= pixels[cswap];
                    pixels[cswap] ^= pixels[cswap+2];
                }
            }
        } else {
            // compressed image
            unsigned int pixelcount = imageHeight * imageWidth;                 // Nuber of pixels in the image
            unsigned int currentpixel   = 0;                                        // Current pixel being read
            unsigned int currentbyte    = 0;                                        // Current byte
            unsigned char * colorbuffer = (unsigned char *)SL_MALLOC(imageBytesPerPel); // Storage for 1 pixel

            do {
                unsigned char chunkheader = 0;                                      // Storage for "chunk" header

                memcpy(&chunkheader, p, sizeof(unsigned char));
                p += sizeof(unsigned char);

                if(chunkheader < 128) {                                         // If the ehader is < 128, it means the that is the number of RAW color packets minus 1
                    // that follow the header
                    chunkheader++;                                              // add 1 to get number of following color values
                    for(short counter = 0; counter < chunkheader; counter++) {  // Read RAW color values
                        memcpy(colorbuffer, p, imageBytesPerPel);
                        p += imageBytesPerPel;
                        // write to memory
                        pixels[currentbyte      ] = colorbuffer[2];     // Flip R and B vcolor values around in the process
                        pixels[currentbyte + 1  ] = colorbuffer[1];
                        pixels[currentbyte + 2  ] = colorbuffer[0];

                        if (imageBytesPerPel == 4) {                        // if its a 32 bpp image
                            pixels[currentbyte + 3] = colorbuffer[3];       // copy the 4th byte
                        }

                        currentbyte += imageBytesPerPel;                    // Increase thecurrent byte by the number of bytes per pixel
                        currentpixel++;                                     // Increase current pixel by 1

                        if(currentpixel > pixelcount) {                     // Make sure we havent read too many pixels
                            Atmosphere::GetResourceLoader()->FreeResource(data);                                // Close file

                            if(colorbuffer != NULL) {                       // If there is data in colorbuffer
                                SL_FREE(colorbuffer);                           // Delete it
                            }

                            if(pixels != NULL) {                        // If there is Image data
                                SL_FREE(pixels);                                // delete it
                            }

                            return false;                                   // Return failed
                        }
                    }
                } else {                                                    // chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
                    chunkheader -= 127;                                     // Subtract 127 to get rid of the ID bit

                    memcpy(colorbuffer, p, imageBytesPerPel);
                    p += imageBytesPerPel;

                    for(short counter = 0; counter < chunkheader; counter++) { // copy the color into the image data as many times as dictated
                        // by the header
                        pixels[currentbyte      ] = colorbuffer[2];     // switch R and B bytes areound while copying
                        pixels[currentbyte + 1  ] = colorbuffer[1];
                        pixels[currentbyte   + 2    ] = colorbuffer[0];

                        if(imageBytesPerPel == 4) {                         // If TGA images is 32 bpp
                            pixels[currentbyte + 3] = colorbuffer[3];       // Copy 4th byte
                        }

                        currentbyte += imageBytesPerPel;                    // Increase current byte by the number of bytes per pixel
                        currentpixel++;                                     // Increase pixel count by 1

                        if(currentpixel > pixelcount) {                     // Make sure we havent written too many pixels
                            Atmosphere::GetResourceLoader()->FreeResource(data);                                // Close file

                            if(colorbuffer != NULL) {                       // If there is data in colorbuffer
                                SL_FREE(colorbuffer);                           // Delete it
                            }

                            if(pixels != NULL) {                        // If there is Image data
                                SL_FREE(pixels);                                // delete it
                            }

                            return false;                                   // Return failed
                        }
                    } // for(counter)
                } // if(chunkheader)
            } while(currentpixel < pixelcount);                             // Loop while there are still pixels left

            if (colorbuffer != NULL) {
                SL_FREE(colorbuffer);
            }

        } // if (imageType == 2)

        Atmosphere::GetResourceLoader()->FreeResource(data);
    }

    return true;
}

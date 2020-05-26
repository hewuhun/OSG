// Copyright 2006-2011 Sundog Software, LLC. All rights reserved worldwide.


#include <FeTriton/TGALoader.h>
#include <FeTriton/ResourceLoader.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace Triton;

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
        TRITON_FREE(pixels);
    }
}

bool TGALoader::Load(const char *filename, ResourceLoader *rl)
{
    if (pixels) {
        TRITON_FREE(pixels);
        pixels = 0;
    }

    char *data;
    unsigned int dataLen;
    if (rl->LoadResource(filename, data, dataLen, false)) {
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

        // Validate header info
        if ( ( imageType != 2 && imageType != 3 && imageType != 10 ) ||
                ( imageWidth == 0 ) || ( imageHeight == 0 ) ||
                ( imageBytesPerPel != 3 && imageBytesPerPel != 4 && imageBytesPerPel != 1) ) {
            // invalid header, bomb out
            rl->FreeResource(data);
            Utils::DebugMsg("Invalid TGA header");
            return false;
        }

        // Allocate the memory for the image size
        pixels = (unsigned char *)TRITON_MALLOC(imageSize);

        // Skip image ident field
        if ( header.ImgIdent > 0 ) {
            p += header.ImgIdent;
        }

        // un-compresses image ?
        if (imageType == 2 || imageType == 3) {
            memcpy(pixels, p, imageSize);

            if (imageType == 2) {
                for (unsigned int cswap = 0; cswap < imageSize; cswap += imageBytesPerPel) {
                    pixels[cswap] ^= pixels[cswap+2];
                    pixels[cswap+2] ^= pixels[cswap];
                    pixels[cswap] ^= pixels[cswap+2];
                }
            }
        }

        rl->FreeResource(data);
    }

    return true;
}

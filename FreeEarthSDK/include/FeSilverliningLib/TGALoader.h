// Copyright (c) 2006-2009 Sundog Software, LLC. All rights reserved worldwide.

/**
    \file TGALoader.h
    \brief Loads a TGA file into a byte array.
 */

#ifndef TGA_LOADER_H
#define TGA_LOADER_H

#include <FeSilverliningLib/MemAlloc.h>

namespace SilverLining
{
/** Loads a TGA file into a byte array. */
class TGALoader : public MemObject
{
public:
/** Default constructor. */
    TGALoader();

/** Virtual destructor. */
    virtual ~TGALoader();

/** Loads the specified TGA graphics file.
   \param filename The full path to the TGA file to load.
   \return True if the operation succeeded.
 */
    bool Load(const char *filename);

/** Returns a pointer to the interleaved RGB or RGBA pixels
   that make up this image. Assumes Load() has been called. */
    unsigned char *GetPixels() {
        return pixels;
    }

/** Returns the width of the image, in pixels. Assumes Load()
   has been called. */
    int GetWidth() const {
        return width;
    }

/** Returns the height of the image, in pixels. Assumes Load()
   has been called. */
    int GetHeight() const {
        return height;
    }

/** Returns the number of bits per pixel. ie, 32 for RGBA images,
   or 24 for RGB images. Assumes Load() has been called. */
    int GetBitsPerPixel() const {
        return bpp;
    }

private:
    unsigned char *pixels;
    int width, height, bpp;
};
}

#endif

#include <ImageDecode.h>

#include <stdio.h>
#include <csetjmp>

#include <jpeglib.h>
#include <png.h>

struct jpegErrorManager 
{
    /* "public" fields */
    struct jpeg_error_mgr pub;
    /* for return to caller */
    jmp_buf setjmp_buffer;
};

char jpegLastErrorMsg[512];
void jpegErrorExit (j_common_ptr cinfo)
{
    /* cinfo->err actually points to a jpegErrorManager struct */
    jpegErrorManager* myerr = (jpegErrorManager*) cinfo->err;
    /* note : *(cinfo->err) is now equivalent to myerr->pub */

    /* output_message is a method to print an error message */
    /*(* (cinfo->err->output_message) ) (cinfo);*/

    /* Create the message */
    ( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);

    /* Jump to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);

}

unsigned char * LoadJPG(unsigned char * buf,unsigned int size,int &w,int &h,int &c,bool invert)
{
    if(!buf)
        return 0;

    struct jpeg_decompress_struct cinfo;
    struct jpegErrorManager jerr;
    JSAMPROW j;
    unsigned int i;

    /* Create and configure decompressor */
    jpeg_create_decompress (&cinfo);
    cinfo.err = jpeg_std_error (&jerr.pub);
    jerr.pub.error_exit = jpegErrorExit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        printf(jpegLastErrorMsg);

        jpeg_destroy_decompress(&cinfo);
        return 0;
    }
    jpeg_mem_src(&cinfo, buf,size);

    /* Read header and prepare for decompression */
    jpeg_read_header (&cinfo, TRUE);
    jpeg_start_decompress (&cinfo);

    w = cinfo.image_width;
    h = cinfo.image_height;

    c = cinfo.num_components;

    unsigned char * data = new unsigned char[w*h*c];

    /* Extract each scanline of the image */
    for (i = 0; i < h; ++i)
    {
        if(invert)
        j = (data +
            ((h - (i + 1)) * w * c));
        else
        j = data + i * w * c;
        jpeg_read_scanlines (&cinfo, &j, 1);
    }

    /* Finish decompression and release memory */
    jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);
    return data;

}

typedef struct
{
    unsigned char* data;
    unsigned int size;
    int offset;
}ImageSource;

//浠庡唴瀛樿鍙朠NG锲剧墖镄勫洖璋冨嚱鏁?
void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    ImageSource* isource = (ImageSource*)png_get_io_ptr(png_ptr);
    if(isource->offset + length <= isource->size)
    {
        memcpy(data, isource->data+isource->offset, length);
        isource->offset += length;
    }
    else
        png_error(png_ptr, "pngReaderCallback failed");
}

//浠庡唴瀛樿鍙?
unsigned char * LoadPNGFormBufRGB(unsigned char* data, const unsigned int dataSize,int &w,int &h,int &c)
{
    unsigned char * m_bgra;
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(png_ptr == 0)
        return 0;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == 0)
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return 0;
    }
    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,0);
    }

    ImageSource imgsource;
    imgsource.data = data;
    imgsource.size = dataSize;
    imgsource.offset = 0;

    png_set_read_fn(png_ptr, &imgsource,pngReadCallback);



    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_ALPHA, 0);

    if(!png_ptr)
        return 0;

    w = info_ptr->width;
    h = info_ptr->height;
    c = info_ptr->channels;


    //濡傛灉瑕佷娇鐢ㄩ€忔槑阃氶亾璇风敤涓嬮溃涓€琛屻€?
    //png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

    png_bytep* row_pointers = png_get_rows(png_ptr,info_ptr);
    m_bgra = new unsigned char[info_ptr->width*info_ptr->height*3];

    int pos=0;
    for(int i=info_ptr->height-1;i>=0;i--)
    {
        for(unsigned int j=0;j<3*info_ptr->width;j+=3)
        {
            m_bgra[pos++] = row_pointers[i][j+0];//r
            m_bgra[pos++] = row_pointers[i][j+1];//g
            m_bgra[pos++] = row_pointers[i][j+2];//b
        }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return m_bgra;
}

unsigned char * LoadPNGFormBufRGBA(unsigned char* data, const unsigned int dataSize,int &w,int &h,int &c)
{
    unsigned char * m_bgra;
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(png_ptr == 0)
        return 0;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == 0)
    {
        png_destroy_read_struct(&png_ptr, 0, 0);
        return 0;
    }
    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,0);
    }

    ImageSource imgsource;
    imgsource.data = data;
    imgsource.size = dataSize;
    imgsource.offset = 0;

    png_set_read_fn(png_ptr, &imgsource,pngReadCallback);


    //濡傛灉瑕佷娇鐢ㄩ€忔槑阃氶亾璇风敤涓嬮溃涓€琛屻€?
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

    if(!png_ptr)
        return 0;

    w = info_ptr->width;
    h = info_ptr->height;
    c = info_ptr->channels;

    png_bytep* row_pointers = png_get_rows(png_ptr,info_ptr);

    m_bgra = new unsigned char[info_ptr->width*info_ptr->height*4];

    int pos=0;
    for(int i=info_ptr->height-1;i>=0;i--)
    {
        for(unsigned int j=0;j<c*info_ptr->width;j+=c)
        {
            //for(int k=0;k<c;k++)
            //m_bgra[pos++] = row_pointers[i][j+k];//r
            if(c==4)
            {
              m_bgra[pos++] = row_pointers[i][j+0];//r
              m_bgra[pos++] = row_pointers[i][j+1];//g
              m_bgra[pos++] = row_pointers[i][j+2];//b
              m_bgra[pos++] = row_pointers[i][j+3];//a
            }
            if(c==2)//
            {
              m_bgra[pos++] = row_pointers[i][j+0];//r
              m_bgra[pos++] = row_pointers[i][j+0];//g
              m_bgra[pos++] = row_pointers[i][j+0];//b
              m_bgra[pos++] = row_pointers[i][j+1];//a
            }
            if(c==3)//
            {
              m_bgra[pos++] = row_pointers[i][j+0];//r
              m_bgra[pos++] = row_pointers[i][j+1];//g
              m_bgra[pos++] = row_pointers[i][j+2];//b
              m_bgra[pos++] = 0xff;//a
            }
        }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return m_bgra;
}



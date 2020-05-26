#ifndef IMAGE_DECODE_H
#define IMAGE_DECODE_H

//浠庡唴瀛樿В镰丳NG镄勫嚱鏁?
//涓夐€氶亾瑙ｇ爜
unsigned char * LoadPNGFormBufRGB(unsigned char* data, const unsigned int dataSize,int &w,int &h,int &c);
//甯﹂€忔槑阃氶亾瑙ｇ爜
unsigned char * LoadPNGFormBufRGBA(unsigned char* data, const unsigned int dataSize,int &w,int &h,int &c);
//浠庡唴瀛樿В镰乯pg锲惧儚
unsigned char * LoadJPG(unsigned char * buf,unsigned int size,int &w,int &h,int &c,bool invert=true);

#endif // IMAGE_DECODE_H

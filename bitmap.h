#ifndef BITMAP_H
#define BITMAP_H


#define R5    0xF800     //
#define R5AX  0x7C00     //
#define G6    0x7E0      //
#define G5    0x3E0      //
#define B5    0x1F       //
#define A1    0x8000     //
#define A8    0xFF000000 // bit flags

#define F16G6 0x1//
#define F16A  0x2//
#define F16X  0x3//
#define F24   0x4//
#define F32A  0x5//
#define F32X  0x6//formats


#define HEADERSIZE 0x7A

#define MIN_HEADERSIZE 54
#define MAX_HEADERSIZE 124


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <math.h>

#define _Throw(X) { fprintf(stderr,"%s LINE: %s FILE: %s",X,__LINE__,__FILE__); return false; }

struct ISize{ uint16_t w; uint16_t h; };

struct pixel {

    pixel()                                             : r(0),g(0),b(0),a(0)         {}
    pixel(uint8_t R,uint8_t G,uint8_t B,uint8_t A=0xff) : r(R),g(G),b(B),a(A)         {}
    pixel(const pixel&x)                                : r(x.r),g(x.g),b(x.b),a(x.a) {}

    void   operator =   (const pixel& x)  { r=x.r; g=x.g; b=x.b; a=x.a; }

    uint8_t r,g,b,a;
};

class bitmap{

    ISize  m_size;
    pixel *m_data;

public:

    bitmap();
    ~bitmap()         { Clear();        }

    pixel*        GetData() { return  m_data; }
    const ISize&  GetSize() { return  m_size; }

    void          Clear();
    bool          LoadFile(const char* path);
private:
    bool          LoadBmp16(uint32_t format,FILE * file,ISize sz);
    bool          LoadBmp  (uint32_t format,FILE * file,ISize sz);

public:
    bool          WriteFile(const char* path);


};

#endif // BITMAP_H

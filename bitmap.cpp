#include "bitmap.h"

bitmap::bitmap(){
    m_data   = NULL;
    m_size.w = m_size.h = 0;
}

void bitmap::Clear(){
    if(m_data) { delete m_data; }
    m_size.w = m_size.h = 0;
}

bool bitmap::LoadFile(const char * path) {

#ifdef _MSC_VER
    FILE * file=NULL;
    if( fopen_s(&file,path,"rb") != 0 ) { _Throw("File Error"); }
#else
    FILE * file=fopen(path,"rb");
    if(!file) { _Throw("File Error"); }
#endif // _MSC_VER

    uint8_t header[MAX_HEADERSIZE+1];
    memset (header,0,MAX_HEADERSIZE);


    if (fread(header,1,MAX_HEADERSIZE,file)<MIN_HEADERSIZE) { fclose(file); _Throw("BMP Read Error");  }

    if (header[0]!='B' || header[1]!='M')                   { fclose(file); _Throw("BMP Header Fail"); }


    int32_t iheadersz=(uint32_t)header[0x0E];

    if( (iheadersz>=MIN_HEADERSIZE-14) && (iheadersz<=MAX_HEADERSIZE) ) {

        ISize sz;

        uint32_t   *sw = (uint32_t*)&header[0x12];
        uint32_t   *sh = (uint32_t*)&header[0x16];
        sz.w =*sw;
        sz.h =*sh;

        uint32_t   *datapos     = (uint32_t*)&header[0x0A];
        uint16_t   *bitspp      = (uint16_t*)&header[0x1C];
        uint32_t   *compression = (uint32_t*)&header[0x1E];


        if(fseek(file,*datapos,SEEK_SET)!=0)          { fclose(file); _Throw("BMP DATAPOS Seek Fail");         }

        if(!( (*compression==0)||(*compression==3) )) { fclose(file); _Throw("BMP Compression Not Supported"); }
        if(*compression==3) {
            if(iheadersz<108)                         { fclose(file); _Throw("BMP header Not Supported");      }

            uint32_t * gm   = (uint32_t*)&header[0x3A];
            uint32_t * am   = (uint32_t*)&header[0x42];
            uint32_t greenM = *gm;
            uint32_t alphaM = *am;

            if(*bitspp==16) {


                if     (greenM==G6) { return LoadBmp16(F16G6,file,sz); }
                else if(alphaM==A1) { return LoadBmp16(F16A,file,sz);  }
                else if(alphaM==0)  { return LoadBmp16(F16X,file,sz);  }
                else                { fclose(file); _Throw("BMP Not Supported"); }

            } else if(*bitspp==32) {

                if(alphaM==A8)      { return LoadBmp(F32A,file,sz);    }
                else if(alphaM==0)  { return LoadBmp(F32X,file,sz);    }
                else                { fclose(file); _Throw("BMP Not Supported");   }

            } else if(*bitspp==24)  { return LoadBmp(F24,file,sz);  }
            else {  fclose(file); _Throw("BMP Not Supported");  }

        } else { return LoadBmp(F24,file,sz); }


    } else {  fclose(file); _Throw("BMP Not Supported"); }

    fclose(file);
    return true;
}

bool Loadpixel16(uint32_t format, uint16_t p, pixel*px) {

    float r(0),g(0),b(0),a(1);

    if(format==F16G6) {

        r= ((p&R5) >>11)/31.0f;
        g= ((p&G6) >>5)/63.0f;
        b= ( p&B5) /31.0f;

    } else if((format==F16A)||(format==F16X)) {

        r= ((p& R5AX) >>10)/31.0f;
        g= ((p& G5)   >>5) /31.0f;
        b= ( p& B5)        /31.0f;

        a= ( format==F16A)?(p&A1)>>15:1.0f;

    } else { return  false; }

    px->r = uint8_t(r*0xFF);
    px->g = uint8_t(g*0xFF);
    px->b = uint8_t(b*0xFF);
    px->a = uint8_t(a*0xFF);

    return true;
}

bool bitmap::LoadBmp16(uint32_t format,FILE * file,ISize sz) {

    uint32_t pxcnt   = 2;
    uint32_t imagesz = sz.h*sz.w*pxcnt;

    uint16_t *data   = new uint16_t[sz.h*sz.w];

    if(fread((uint8_t*)data,1,imagesz,file)!=imagesz) {
        delete [] data;
        fclose(file); _Throw("BMP data Read Fail");
    }

    m_data= new pixel[sz.h*sz.w];

    for(long  H=0,h=(sz.h-1); h>=0; H++,h--) {

        for(long i=0,w=0; w<(sz.w); w++,i++) {

            long pos=((h)*sz.w+i);
            if(!Loadpixel16(format,data[pos],&m_data[H*sz.w+w])) {
                delete [] data; delete [] m_data;
                m_data=NULL;
                fclose(file);
                _Throw("BMP LOADPIXEL16 Fail");
            }
        }

    }
    delete [] data;

    m_size.h=sz.h;
    m_size.w=sz.w;

    return true;
}

bool bitmap::LoadBmp(uint32_t format,FILE * file,ISize sz) {

    uint32_t pxcnt   = (format==F24)?3:4;
    uint32_t imagesz = sz.h*sz.w*pxcnt;

    uint8_t *data  = new uint8_t[imagesz];

    if(fread(data,1,imagesz,file)!=imagesz) {
        delete [] data;
        fclose(file); _Throw("BMP data Read Fail");
    }

    m_data= new pixel[sz.h*sz.w];

    for(long  H=0,h=(sz.h-1); h>=0; H++,h--) {

        for(long i=0,w=0; w<(sz.w); w++,i+=pxcnt) {

            long pos=((h)*sz.w*pxcnt);
            m_data[H*sz.w+w].b = data[pos+i];
            m_data[H*sz.w+w].g = data[pos+i+1];
            m_data[H*sz.w+w].r = data[pos+i+2];
            m_data[H*sz.w+w].a = (format==F32A)?data[pos+i+3]:0xFF;
        }

    }

    delete [] data;
    m_size.h=sz.h;
    m_size.w=sz.w;

    return true;
}

bool bitmap::WriteFile(const char* path){

    if(!m_data || (m_size.h==0) || (m_size.w==0) ) { _Throw("unset image"); }

    uint32_t datasize  = HEADERSIZE;
    uint32_t imagepxsz = m_size.w * m_size.h;

    datasize += (imagepxsz*4);

    uint8_t *imgdata = new uint8_t[datasize+1];
    for(uint32_t i=0; i <= datasize; imgdata[i]=0,i++);

    imgdata[0]='B'; imgdata[1]='M';

    uint32_t* ds    = (uint32_t*)&imgdata[2];    *ds=datasize;

    uint16_t* r     = (uint16_t*)&imgdata[6];    *r=0;
    r               = (uint16_t*)&imgdata[8];    *r=0;

    uint32_t* sadd  = (uint32_t*)&imgdata[0xA];  *sadd=HEADERSIZE;

    uint32_t* hsz   = (uint32_t*)&imgdata[0xE];  *hsz=0x6C;

    uint32_t* wdt   = (uint32_t*)&imgdata[0x12]; *wdt=m_size.w;

    uint32_t* ht    = (uint32_t*)&imgdata[0x16]; *ht=m_size.h;

    uint16_t* pns   = (uint16_t*)&imgdata[0x1A]; *pns=1;

    uint16_t* bpp   = (uint16_t*)&imgdata[0x1C]; *bpp=32;

    uint32_t* cm    = (uint32_t*)&imgdata[0x1E]; *cm=3;

    uint32_t* isz   = (uint32_t*)&imgdata[0x22]; *isz=32;

    uint32_t* hppm  = (uint32_t*)&imgdata[0x26]; *hppm=2835;/* 72dpi * 39.3701(1 meter)*/

    uint32_t* vppm  = (uint32_t*)&imgdata[0x2A]; *vppm=2835;/* 72dpi * 39.3701(1 meter)*/

    uint32_t* cpn   = (uint32_t*)&imgdata[0x2E]; *cpn=0;/*default*/

    uint32_t* iclr  = (uint32_t*)&imgdata[0x32]; *iclr=0;

    uint32_t* rbf   = (uint32_t*)&imgdata[0x36]; *rbf=0xFF0000;

    uint32_t* gbf   = (uint32_t*)&imgdata[0x3A]; *gbf=0xFF00;

    uint32_t* bbf   = (uint32_t*)&imgdata[0x3E]; *bbf=0xFF;

    uint32_t* abf   = (uint32_t*)&imgdata[0x42]; *abf=0xFF000000;

    uint8_t* datapos=imgdata+HEADERSIZE;

    pixel curr;
    for(uint32_t  h=m_size.h,i=0; h>0; h--) {

        for(uint32_t  w=0; w<m_size.w; w++,i+=4) {
            curr         = m_data[ (h*m_size.w) - (m_size.w-w)  ];
            datapos[i]   = curr.b;
            datapos[i+1] = curr.g;
            datapos[i+2] = curr.r;
            datapos[i+3] = curr.a;
        }

    }


#ifdef _MSC_VER
    FILE * file=NULL;
    if( fopen_s(&file,path,"wb") != 0 ) { delete [] imgdata; _Throw("File Error"); }
#else
    FILE * file=fopen(path,"wb");
    if(!file) { delete [] imgdata; _Throw("File Error"); }
#endif // _MSC_VER

    fwrite(imgdata,datasize,1,file);

    delete [] imgdata;
    return true;
}

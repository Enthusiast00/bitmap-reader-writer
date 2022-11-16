
#include "bitmap.h"

#include "window.h"

window::window() {

    m_hdc      = NULL;
    m_bmhdc    = NULL;
    m_imagebm  = NULL;

    m_hwnd     = NULL;
    m_instance = NULL;

}
static inline float lerp( const float& from,const float& too,float lerp_tm) {
    return ((too*lerp_tm) + ((1.0f-lerp_tm)*from));
}

static bool Data2Bitmap(HDC hdc,HBITMAP * bitmap,const ISize& size, uint8_t* data,COLORREF background ) {

    if(!data) { _Throw(" data ");      }

    uint32_t *bitmapbits    = new uint32_t[size.w*size.h+1];

    uint32_t  datapos(0),bmpos(0),currheight(0);

    for(uint16_t h=0; h<size.h; h++) {

        currheight = h*size.w;
        for(uint16_t w=0; w<size.w; w++,datapos+=4) {

            bmpos     = currheight ?currheight-(size.w-w):w;

            float scl(data[datapos+3] / 255.0f );

            uint8_t r= uint8_t ( lerp( GetRValue(background),data[datapos]   ,scl) );
            uint8_t g= uint8_t ( lerp( GetGValue(background),data[datapos+1] ,scl) );
            uint8_t b= uint8_t ( lerp( GetBValue(background),data[datapos+2] ,scl) );

            bitmapbits[bmpos]=RGB(b,g,r);
        }
    }

    (*bitmap)    = CreateCompatibleBitmap ( hdc,size.w,size.h-1);
    if((*bitmap) == NULL) { _Throw("BMP create fail "); }

    SetBitmapBits((*bitmap),size.w*(size.h-1)*4,bitmapbits);

    delete [] bitmapbits;

    return true;
}


bool window::Initialize(bitmap *img){

    if(!img  || !img->GetData() ){ _Throw("bitmaploader"); }

    m_instance = GetModuleHandle(NULL);

    WNDCLASS wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_instance;
    wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground =(HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = "Bitmap-loader";

    if(!::RegisterClass(&wc)){ _Throw("RegisterClass - Failed \n"); }

    m_hwnd = CreateWindow("Bitmap-loader","Bitmap-loader",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          D_WIDTH,D_HEIGHT,
                          0,0,m_instance,0);

    if(m_hwnd == 0){ _Throw("CreateWindow Error \n"); }

    m_hdc      = GetDC(m_hwnd);

    m_bmhdc = CreateCompatibleDC(m_hdc);
    if(!m_bmhdc) { _Throw("bitmap dc"); }


    bool pass=Data2Bitmap(m_hdc,&m_imagebm,img->GetSize(), (uint8_t*)img->GetData(),RGB(0,0,0));
    if(!m_imagebm || !pass) { _Throw("bitmap "); }
    m_imagewidth  = img->GetSize().w;
    m_imageheight = img->GetSize().h;

    ShowWindow(m_hwnd, SW_NORMAL);
    UpdateWindow(m_hwnd);


    return true;
}

int window::Start(){

    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    Draw();
    while(GetMessage(&msg, 0, 0, 0) ){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Draw();

    }
    return msg.wParam;

}

void window::Draw(){

    HBITMAP dbmp   = (HBITMAP)SelectObject( m_bmhdc, m_imagebm);

    RECT size;
    ZeroMemory(&size, sizeof(RECT));

    GetClientRect(m_hwnd,&size);

    int32_t winwidth   =  int32_t( size.right-size.left );
    int32_t winheight  =  int32_t( size.bottom-size.top );

    int32_t x = winwidth  > m_imagewidth  ? (winwidth - m_imagewidth )/2 :0;
    int32_t y = winheight > m_imageheight ? (winheight- m_imageheight)/2 :0;

    BitBlt(m_hdc,x,y,m_imagewidth,m_imageheight,m_bmhdc,0,0, SRCCOPY);

}

LRESULT CALLBACK window::WndProc(HWND   hwnd,UINT   msg,WPARAM wParam,LPARAM lParam){
    switch( msg )
    {

    case WM_KEYDOWN:
        if( wParam == VK_ESCAPE )
            DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}


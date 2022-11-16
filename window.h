#ifndef WINDOW_H
#define WINDOW_H

#define D_WIDTH  800
#define D_HEIGHT 400

#include <windows.h>

class bitmap;

class window {


    HWND           m_hwnd;
    HINSTANCE      m_instance;

    HDC            m_hdc;
    HDC            m_bmhdc;
    HBITMAP        m_imagebm;

    unsigned short m_imagewidth;
    unsigned short m_imageheight;

public:
    window();
    bool Initialize(bitmap *img);
    int Start();

private:

    void Draw();

    static LRESULT CALLBACK WndProc(HWND   windowHandle,UINT   msg,WPARAM wParam,LPARAM lParam);

};

#endif // WINDOW_H

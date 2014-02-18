/*
  Jun 30, 2004 carlc
  This is a class for "animated bitmaps".  Basically the idea is that you get easy low level
  access do a DIB and its meant to be drawn in frames.

  This is only meant for SCREEN purposes.
*/


#pragma once


#include <windows.h>
#include "blob.h"
#include "colorframework.h"

using namespace Colors;


class AnimBitmap
{
public:
  AnimBitmap() :
    m_y(0),
    m_x(0),
    m_bmp(0),
    m_pbuf(0)
  {
    // store our offscreen hdc
    HDC hscreen = ::GetDC(0);
    m_offscreen = CreateCompatibleDC(hscreen);
    ReleaseDC(0, hscreen);
  }

  ~AnimBitmap()
  {
    DeleteDC(m_offscreen);
    if(m_bmp)
    {
      DeleteObject(m_bmp);
    }
  }

  CSize GetSize() const
  {
    return CSize(m_x, m_y);
  }

  long GetWidth() const
  {
    return m_x;
  }

  long GetHeight() const
  {
    return m_y;
  }

  // MUST be called at least once.  This will allocate the bmp object.
  bool SetSize(long x, long y)
  {
    bool r = false;

    if((x != m_x) || (y != m_y))
    {
      // delete our current bmp.
      if(m_bmp)
      {
        DeleteObject(m_bmp);
        m_bmp = 0;
      }

      BITMAPINFO bi = {0};

      bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
      bi.bmiHeader.biPlanes = 1;// must be 1
      bi.bmiHeader.biBitCount = 32;// for RGBQUAD
      bi.bmiHeader.biCompression = BI_RGB;
      bi.bmiHeader.biSizeImage = 0;// dont need to specify because its uncompressed
      bi.bmiHeader.biXPelsPerMeter = 0;
      bi.bmiHeader.biYPelsPerMeter = 0;
      bi.bmiHeader.biClrUsed  = 0;
      bi.bmiHeader.biClrImportant = 0;
      bi.bmiHeader.biWidth = max(x,1);
      bi.bmiHeader.biHeight = -(max(y,1));

      m_bmp = CreateDIBSection(m_offscreen, &bi, DIB_RGB_COLORS, (void**)&m_pbuf, 0, 0);
      bi.bmiHeader.biHeight = (max(y,1));
      if(m_bmp)
      {
        r = true;
        m_x = bi.bmiHeader.biWidth;
        m_y = bi.bmiHeader.biHeight;
        SelectObject(m_offscreen, m_bmp);
      }
    }
    return r;
  }

  // commits the changes
  bool Commit()
  {
    return true;
  }

  bool BeginDraw()
  {
    GdiFlush();
    return true;
  }

  HDC GetDC()
  {
    return m_offscreen;
  }

  // no boundschecking for speed.
  void SetPixel(long x, long y, RgbPixel c)
  {
    ATLASSERT(x >= 0);
    ATLASSERT(y >= 0);
    ATLASSERT(y < m_y);
    ATLASSERT(x < m_x);
    //y = m_y - y;
    m_pbuf[x + (y * m_x)] = c;
  }

  // xright is NOT drawn.
  void HLine(long x1, long x2, long y, RgbPixel c)
  {
    long xleft = min(x1, x2);
    long xright = max(x1, x2);
    RgbPixel* pbuf = &m_pbuf[(y * m_x) + xleft];
    while(xleft != xright)
    {
      *pbuf = c;
      pbuf ++;
      xleft ++;
    }
  }

  void VLine(long x, long y1, long y2, RgbPixel c)
  {
    long ytop = min(y1, y2);
    long ybottom = max(y1, y2);
    RgbPixel* pbuf = &m_pbuf[(ytop * m_x) + x];
    while(ytop != ybottom)
    {
      *pbuf = c;
      pbuf += m_x;
      ytop ++;
    }
  }

  // b and r are not drawn
  void Rect(long l, long t, long r, long b, RgbPixel c)
  {
    RgbPixel* pbuf = &m_pbuf[(t * m_x) + l];
    long h = r - l;// horizontal size
    // fill downwards
    while(t != b)
    {
      // draw a horizontal line
      for(long i = 0; i < h; i ++)
      {
        pbuf[i] = c;
      }
      pbuf += m_x;
      t ++;
    }
  }

  bool SetPixelSafe(long x, long y, RgbPixel c)
  {
    bool r = false;
    if(x > 0 && y > 0 && x < m_x && y < m_y)
    {
      m_pbuf[x + (y * m_x)] = c;
      r = true;
    }
    return r;
  }

  RgbPixel GetPixel(long x, long y)
  {
    return m_pbuf[x + (y * m_x)];
  }

  bool GetPixelSafe(RgbPixel& out, long x, long y)
  {
    bool r = false;
    if(x > 0 && y > 0 && x < m_x && y < m_y)
    {
      out = m_pbuf[x + (y * m_x)];
      r = true;
    }
    return r;
  }

  void Fill(RgbPixel c)
  {
    RgbPixel* pDest = m_pbuf;
    long n = m_x * m_y;
    for(long i = 0; i < n; i ++)
    {
      pDest[i] = c;
    }
  }

  bool StretchBlit(AnimBitmap& dest, long destx, long desty, long destw, long desth, long srcx, long srcy, long srcw, long srch)
  {
    int r = StretchBlt(
      dest.m_offscreen, destx, desty, destw, desth,
      m_offscreen, srcx, srcy, srcw, srch, SRCCOPY);
    return r != 0;
  }

  bool StretchBlit(AnimBitmap& dest, long x, long y, long w, long h)
  {
    int r = StretchBlt(dest.m_offscreen, x, y, w, h, m_offscreen, 0, 0, m_x, m_y, SRCCOPY);
    return r != 0;
  }

  bool StretchBlit(HDC hDest, long x, long y, long w, long h)
  {
    int r = StretchBlt(hDest, x, y, w, h, m_offscreen, 0, 0, m_x, m_y, SRCCOPY);
    return r != 0;
  }

  bool Blit(HDC hDest, long x, long y)
  {
    int r = BitBlt(hDest, x, y, x + m_x, y + m_y, m_offscreen, 0, 0, SRCCOPY);
    return r != 0;
  }

  bool Blit(AnimBitmap& dest, long x, long y)
  {
    int r = BitBlt(dest.m_offscreen, x, y, x + m_x, y + m_y, m_offscreen, 0, 0, SRCCOPY);
    return r != 0;
  }

  bool _DrawText(const char* s, long x, long y)
  {
    CRect rc(x, y, m_x, m_y);
    DrawText(m_offscreen, s, static_cast<int>(strlen(s)), &rc, DT_NOCLIP);
    return true;
  }

private:
  long m_x;
  long m_y;
  HDC m_offscreen;
  HBITMAP m_bmp;
  RgbPixel* m_pbuf;
};


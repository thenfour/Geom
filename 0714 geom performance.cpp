

#include "stdafx.h"
#include "fps.h"
#include "animbitmap.h"
#include "geom.h"
#include "gdiplus.h"

#pragma comment(lib, "gdiplus.lib")

HBRUSH hbr;
CAppModule _Module;
AnimBitmap bmp;
long TestID = 0;
const long TID_Fill = 0;
const long TID_GDICircle = 1;
const long TID_GDIPlusCircle = 2;
const long TID_FilledCircleG = 3;
const long TID_FilledCircleAAG = 5;
const long TID_DonutG = 7;
const long TID_DonutAAG = 8;

Gdiplus::Graphics* graphics = 0;
Gdiplus::SolidBrush* bluePen = 0;
FPS f;


void DontOptimizeOut(...)
{
}

/*
  Integer math color mixing function
*/
inline RgbPixel MixColorsInt(long fa, long fmax, RgbPixel ca, RgbPixel cb)
{
  BYTE r, g, b;
  long fmaxminusfa = fmax - fa;
  r = static_cast<BYTE>(((fa * R(ca)) + (fmaxminusfa * R(cb))) / fmax);
  g = static_cast<BYTE>(((fa * G(ca)) + (fmaxminusfa * G(cb))) / fmax);
  b = static_cast<BYTE>(((fa * B(ca)) + (fmaxminusfa * B(cb))) / fmax);
  return MakeRgbPixel(r,g,b);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_CHAR:
    {
      char c = static_cast<char>(wParam);
      f.ResetTotal();
      switch(c)
      {
      case '0':
        TestID = TID_Fill;
        break;
      case '1':
        TestID = TID_GDICircle;
        break;
      case '2':
        TestID = TID_GDIPlusCircle;
        break;
      case '3':
        TestID = TID_FilledCircleG;
        break;
      case '4':
        TestID = TID_FilledCircleAAG;
        break;
      case '5':
        TestID = TID_DonutG;
        break;
      case '6':
        TestID = TID_DonutAAG;
        break;
      }
      return 0;
    }
  case WM_DESTROY:
    PostQuitMessage(0);
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  case WM_PAINT:
    PAINTSTRUCT ps;
    BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    return 0;
  case WM_SIZE:
    bmp.SetSize(LOWORD(lParam), HIWORD(lParam));
    if(graphics)
    {
      delete graphics;
    }
    graphics = new Gdiplus::Graphics(bmp.GetDC());
    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    return 0;
  case WM_ERASEBKGND:
    return 0;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

class Test
{
public:
  void DonutAA2_SetAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
  {
    //bmp.SetPixel(cx+x, cy+y, MixColorsInt(f, fmax, MakeRgbPixel(255,255,255), bmp.GetPixel(cx+x, cy+y)));
    //bmp.SetPixel(cx+x, cy-y-1, MixColorsInt(f, fmax, MakeRgbPixel(255,255,255), bmp.GetPixel(cx+x, cy-y-1)));
    //bmp.SetPixel(cx-x-1, cy+y, MixColorsInt(f, fmax, MakeRgbPixel(255,255,255), bmp.GetPixel(cx-x-1, cy+y)));
    //bmp.SetPixel(cx-x-1, cy-y-1, MixColorsInt(f, fmax, MakeRgbPixel(255,255,255), bmp.GetPixel(cx-x-1, cy-y-1)));
    bmp.SetPixel(cx+x, cy+y, MixColorsInt(2, 10, MakeRgbPixel(255,0,0), bmp.GetPixel(cx+x, cy+y)));
    bmp.SetPixel(cx+x, cy-y-1, MixColorsInt(2, 10, MakeRgbPixel(255,0,0), bmp.GetPixel(cx+x, cy-y-1)));
    bmp.SetPixel(cx-x-1, cy+y, MixColorsInt(2, 10, MakeRgbPixel(255,0,0), bmp.GetPixel(cx-x-1, cy+y)));
    bmp.SetPixel(cx-x-1, cy-y-1, MixColorsInt(2, 10, MakeRgbPixel(255,0,0), bmp.GetPixel(cx-x-1, cy-y-1)));
  }
  void DonutAAG_Hline(long x1, long x2, long y)
  {
    //bmp.HLine(x1, x2+1, y, MakeRgbPixel(255,255,255));
    long xleft = min(x1, x2);
    long xright = max(x1, x2)+1;
    while(xleft != xright)
    {
      bmp.SetPixel(xleft, y, MixColorsInt(2, 10, MakeRgbPixel(255,255,255), bmp.GetPixel(xleft, y)));
      xleft ++;
    }
  }
};


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  WNDCLASS wc = {0};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "x";
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  RegisterClass(&wc);

  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  bluePen = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255));

  HWND hWnd = CreateWindow("x", "", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 400, 400, 0, 0, 0, 0);

  MSG msg;
  f.SetRecalcInterval(0.2);
  bool bQuit = false;
  Test t;// needed to satisfy callback requirements

  hbr = CreateSolidBrush(RGB(80,80,80));

  while(!bQuit)
  {
    if(PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
    {
      if(GetMessage(&msg, 0, 0, 0))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else
      {
        bQuit = true;
      }
    }
    else
    {
      f.OnFrame();
      bmp.BeginDraw();

      std::string s = f.GetAvgFPSString();
      s.append("fps\r\n");

      switch(TestID)
      {
      case TID_Fill:
        s.append("TID_Fill");
        bmp.Fill(MakeRgbPixel(0,0,0));
        break;
      case TID_GDICircle:
        {
          s.append("TID_GDICircle");
          RECT rc;
          GetClientRect(hWnd, &rc);
          bmp.Fill(MakeRgbPixel(0,0,0));
          if(rc.right > 10 && rc.bottom > 10)
          {
            long rout = (min(rc.bottom, rc.right) / 2) - 3;
            long rin = rout / 3;
            rout = rin;
            long cx = rc.right / 2;
            long cy = rc.bottom / 2;
            //bmp.Fill(MakeRgbPixel(0,0,0));
            Ellipse(bmp.GetDC(), cx-rout-1, cy-rout-1, cx+rout+1,cy+rout+1);
          }
          break;
        }
      case TID_GDIPlusCircle:
        {
          s.append("TID_GDIPlusCircle");
          // Create a Pen object.
          RECT rc;
          GetClientRect(hWnd, &rc);
          bmp.Fill(MakeRgbPixel(0,0,0));
            long rout = (min(rc.bottom, rc.right) / 2) - 3;
            long rin = rout / 3;
            rout = rin;
          long cx = rc.right / 2;
          long cy = rc.bottom / 2;
          // Draw the ellipse.
          graphics->FillEllipse(bluePen, cx-rout-1, cy-rout-1, rout+rout+1, rout+rout+1);
          break;
        }
      case TID_FilledCircleG:
        {
          s.append("TID_FilledCircleG");
          RECT rc;
          GetClientRect(hWnd, &rc);
          bmp.Fill(MakeRgbPixel(0,0,0));
          if(rc.right > 10 && rc.bottom > 10)
          {
            long rout = (min(rc.bottom, rc.right) / 2) - 3;
            long rin = rout / 3;
            rout = rin;
            FilledCircleG(rc.right / 2, rc.bottom / 2, rout,
              &t, Test::DonutAAG_Hline);
          }
          break;
        }
      case TID_FilledCircleAAG:
        {
          s.append("TID_FilledCircleAAG");
          RECT rc;
          GetClientRect(hWnd, &rc);
          bmp.Fill(MakeRgbPixel(0,0,0));
          if(rc.right > 10 && rc.bottom > 10)
          {
            long rout = (min(rc.bottom, rc.right) / 2) - 3;
            long rin = rout / 3;
            rout = rin;
            FilledCircleAAG(rc.right / 2, rc.bottom / 2, rout,
              &t, Test::DonutAAG_Hline,
              &t, Test::DonutAA2_SetAlphaPixel);
          }
          break;
        }
      case TID_DonutG:
        {
          s.append("TID_DonutG");
          RECT rc;
          GetClientRect(hWnd, &rc);
          bmp.Fill(MakeRgbPixel(0,0,0));
          if(rc.right > 10 && rc.bottom > 10)
          {
            long rout = (min(rc.bottom, rc.right) / 2) - 3;
            long rin = rout / 3;
            DonutG(rc.right / 2, rc.bottom / 2, rin, rout-rin,
              &t, Test::DonutAAG_Hline);
          }
          break;
        }
      case TID_DonutAAG:
        {
          s.append("TID_DonutAAG");
          RECT rc;
          GetClientRect(hWnd, &rc);
          bmp.Fill(MakeRgbPixel(0,0,0));
          if(rc.right > 10 && rc.bottom > 10)
          {
            long rout = (min(rc.bottom, rc.right) / 2) - 3;
            long rin = rout / 3;
            DonutAAG(rc.right / 2, rc.bottom / 2, rin, rout-rin,
              &t, Test::DonutAAG_Hline,
              &t, Test::DonutAA2_SetAlphaPixel);
          }
          break;
        }
      }

      bmp.Commit();
      bmp._DrawText(s.c_str(), 0, 0);
      HDC h = GetDC(hWnd);
      bmp.Blit(h, 0, 0);
      ReleaseDC(hWnd, h);
    }
  }

  if(graphics) delete graphics;
  if(bluePen) delete bluePen;

  DeleteObject(hbr);
  Gdiplus::GdiplusShutdown(gdiplusToken);

	return 0;
}

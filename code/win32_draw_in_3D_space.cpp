/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Alexander Baskakov $
   ======================================================================== */
#include <windows.h>
#include <stdint.h>
#include <utility>
#include <math.h>

//#include "../..//win32_code_templates/win32codetemplates.h"
#include "../../win32_code_templates/win32codetemplates.cpp"

#define global_variable static;
#define internal_func static;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef i32 bool32;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef float real32;
typedef double real64;

#define TARGET_CLIENT_RECT_WIDTH 900
#define TARGET_CLIENT_RECT_HEIGHT 600

global_variable bool GlobalRunning = false;

global_variable i32 GlobalClientRectWidth = 0;
global_variable i32 GlobalClientRectHeight = 0;

global_variable HWND GlobalWindowHandle = { };
global_variable HDC GlobalDCHandle = { };
global_variable BITMAPINFO BitmapInfo = { };
global_variable void* GlobalBackbufferMemory = 0;

internal_func void
Win32_GetWindowRectDim(i32* WindowRectWidth, i32* WindowRectHeight)
{
    RECT Rect = { };
    GetWindowRect(GlobalWindowHandle, &Rect);
    *WindowRectWidth = Rect.right - Rect.left;
    *WindowRectHeight = Rect.bottom - Rect.top;
}

internal_func void
Win32_GetClientRectDim(i32* ClientRectWidth, i32* ClientRectHeight)
{
    RECT Rect = { };
    GetClientRect(GlobalWindowHandle, &Rect);
    *ClientRectWidth = Rect.right - Rect.left;
    *ClientRectHeight = Rect.bottom - Rect.top;
}

internal_func void
Win32_ResizeClientRectToTargetRes(i32 TargetWidth, i32 TargetHeight)
{
    i32 CurrentClientRectWidth = 0;
    i32 CurrentClientRectHeight = 0;

    i32 CurrentWindowRectWidht = 0;
    i32 CurrentWindowRectHeight = 0;

    Win32_GetClientRectDim(&CurrentClientRectWidth, &CurrentClientRectHeight);
    Win32_GetWindowRectDim(&CurrentWindowRectWidht, &CurrentWindowRectHeight);

    i32 ClientRectOffsetWidth = CurrentWindowRectWidht - CurrentClientRectWidth;
    i32 ClientRectOffsetHeight = CurrentWindowRectHeight - CurrentClientRectHeight;

    MoveWindow(GlobalWindowHandle, 0, 0 ,
               TargetWidth + ClientRectOffsetWidth,
               TargetHeight + ClientRectOffsetHeight,
               TRUE);  
}

internal_func void
Win32_DrawPixelToBitmapXYInvertedY(void* DestBitmapMemory, int ClientRectWidth,
                             int DestX, int DestY, Pixel32RGB* Pixel)
{
    ui32 TargetX = DestX;
    ui32 TargetY = GlobalClientRectHeight - DestY - 1;
    Win32_DrawPixelToBitmap(DestBitmapMemory, ClientRectWidth, TargetX, TargetY, Pixel);
}

struct v3{
    i32 x;
    i32 y;
    i32 z;
};

struct triangle{

    v3 a;
    v3 b;
    v3 c;
};

v3 &operator+(v3 Point, ui32 Value)
{
    Point.x += Value;
    Point.y += Value;
    return Point;
}

internal_func void
Win32_DrawLineXY(v3 PointA, v3 PointB, Pixel32RGB *Pixel)
{
    i32 x1 = PointA.x;
    i32 y1 = PointA.y;

    i32 x2 = PointB.x;
    i32 y2 = PointB.y;
    
    
    const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
    if(steep)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
 
    if(x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
 
    const float dx = x2 - x1;
    const float dy = fabs(y2 - y1);
 
    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;
 
    const int maxX = (int)x2;
 
    for(int x=(int)x1; x<=maxX; x++)
    {
        if(steep)
        {
            Win32_DrawPixelToBitmapXYInvertedY(GlobalBackbufferMemory, GlobalClientRectWidth,
                                               y, x, Pixel);
        }
        else
        {
            Win32_DrawPixelToBitmapXYInvertedY(GlobalBackbufferMemory, GlobalClientRectWidth,
                                               x, y, Pixel);
        }
 
        error -= dy;
        if(error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

LRESULT CALLBACK
WindowMessageHandlerProcedure(HWND WindowHandle, UINT Message,
           WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = { };

    switch(Message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            BeginPaint(WindowHandle, &PaintStruct);

            

            EndPaint(WindowHandle, &PaintStruct);
            OutputDebugStringA("WM_PAINT\n");
        }break;
        
        case WM_SIZE:
        {
            Win32_GetClientRectDim(&GlobalClientRectWidth, &GlobalClientRectHeight);
            OutputDebugStringA("WM_SIZE\n");
        }break;

        case WM_CLOSE:
        {
            GlobalRunning = false;
        }break;
        
        default:
        {
            Result = DefWindowProc(WindowHandle, Message, wParam, lParam);
        }break;
    }

    return Result;
}

int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
        LPSTR CommandLine, int LineArgs)
{   
    WNDCLASSA WindowClass = { };
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = WindowMessageHandlerProcedure;
    WindowClass.hInstance = WindowInstance;
    WindowClass.hCursor = 0;
    WindowClass.lpszClassName = "Draw3D";

    RegisterClass(&WindowClass);
    
    
    GlobalWindowHandle = CreateWindowEx(0,
                                  WindowClass.lpszClassName,
                                  "Win32Draw3D",
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  NULL,
                                  NULL,
                                  WindowInstance,
                                  NULL);

    if(GlobalWindowHandle)
    {
        Win32_ResizeClientRectToTargetRes(TARGET_CLIENT_RECT_WIDTH,TARGET_CLIENT_RECT_HEIGHT);
        Win32_GetClientRectDim(&GlobalClientRectWidth, &GlobalClientRectHeight);

        GlobalBackbufferMemory = Win32_GetBitmapMemory(&BitmapInfo, GlobalBackbufferMemory, 4,
                              GlobalClientRectWidth, GlobalClientRectHeight);

        GlobalDCHandle = GetDC(GlobalWindowHandle);

        
        ShowWindow(GlobalWindowHandle, LineArgs);
        GlobalRunning = true;

        //Further cube points
        v3 Point1;
        Point1.x = 10;
        Point1.y = 20;
        Point1.z = 50;

        v3 Point2;
        Point2.x = 10;
        Point2.y = 70;
        Point2.z = 50;

        v3 Point3;
        Point3.x = 60;
        Point3.y = 70;
        Point3.z = 50;

        v3 Point4;
        Point4.x = 60;
        Point4.y = 20;
        Point4.z = 50;

              
        v3 Point5;
        Point5.x = 30;
        Point5.y = 5;
        Point5.z = 0;

        v3 Point6;
        Point6.x = 30;
        Point6.y = 55;
        Point6.z = 0;

        v3 Point7;
        Point7.x = 90;
        Point7.y = 55;
        Point7.z = 0;

        v3 Point8;
        Point8.x = 90;
        Point8.y = 5;
        Point8.z = 0;

        
        
        while(GlobalRunning)
        {
            MSG Message = { };
            while(PeekMessage(&Message, GlobalWindowHandle, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Message);
                 DispatchMessage(&Message);
            }

            Pixel32RGB Pixel = {255};
            ui32 PointOffset = 200;

            Win32_DrawLineXY(Point1 + PointOffset, Point2 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point2 + PointOffset, Point3 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point3 + PointOffset, Point4 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point4 + PointOffset, Point1 + PointOffset, &Pixel);

            Win32_DrawLineXY(Point5 + PointOffset, Point6 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point6 + PointOffset, Point7 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point7 + PointOffset, Point8 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point8 + PointOffset, Point5 + PointOffset, &Pixel);

            Win32_DrawLineXY(Point1 + PointOffset, Point5 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point2 + PointOffset, Point6 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point3 + PointOffset, Point7 + PointOffset, &Pixel);
            Win32_DrawLineXY(Point4 + PointOffset, Point8 + PointOffset, &Pixel);

            
            
            Win32_DrawDIBSectionToScreen(&GlobalDCHandle, 0, 0, GlobalClientRectWidth, GlobalClientRectHeight,
                                         0, 0, GlobalClientRectWidth, GlobalClientRectHeight,
                                         GlobalBackbufferMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }
        
        ReleaseDC(GlobalWindowHandle, GlobalDCHandle);
        Win32_ReleaseBitmapMemory(GlobalBackbufferMemory); 
    }

    return 0;
}


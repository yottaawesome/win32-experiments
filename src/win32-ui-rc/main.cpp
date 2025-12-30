// Based on https://learn.microsoft.com/en-us/windows/win32/gdi/using-brushes
// This sample appears to have bugs with it. The original sample specifies a
// GetStrLngth() and RetrieveWidth() functions aren't actually used anywhere.
// I've copied these two functions anyway.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "resource.h"

import std;

int GetStrLngth(LPTSTR cArray)
{
    int i = 0;
    while (cArray[i++] != 0);
    return i - 1;
}

DWORD RetrieveWidth(LPTSTR cArray, int iLength)
{
    int i, iTmp;
    double dVal, dCount;

    dVal = 0.0;
    dCount = (double)(iLength - 1);
    for (i = 0; i < iLength; i++)
    {
        iTmp = cArray[i] - 0x30;
        dVal = dVal + (((double)iTmp) * pow(10.0, dCount--));
    }
    return (DWORD)dVal;
}

auto BrushDlgProc(
	HWND hdlg, 
	UINT message, 
	WPARAM wParam,
	LPARAM lParam
) -> INT_PTR
{
    static HWND hwndGrid;       // grid-window control  
    static HWND hwndBrush;      // pattern-brush control  
    static RECT rctGrid;        // grid-window rectangle  
    static RECT rctBrush;       // pattern-brush rectangle  
    static UINT bBrushBits[8];  // bitmap bits  
    static RECT rect[64];       // grid-cell array  
    static HBITMAP hbm;         // bitmap handle  
    HBRUSH hbrush;              // current brush  
    HBRUSH hbrushOld;           // default brush  
    HRGN hrgnCell;              // test-region handle  
    HDC hdc;                    // device context (DC) handle  
    int x, y, deltaX, deltaY;   // drawing coordinates  
    POINTS ptlHit;              // mouse coordinates  
    int i;                      // count variable  

    switch (message)
    {
        case WM_INITDIALOG:
            // Retrieve a window handle for the grid-window and  
            // pattern-brush controls.  
            hwndGrid = GetDlgItem(hdlg, IDD_GRID);
            hwndBrush = GetDlgItem(hdlg, IDD_RECT);

            // Initialize the array of bits that defines the  
            // custom brush pattern with the value 1 to produce a  
            // solid white brush.  
            for (i = 0; i < 8; i++)
                bBrushBits[i] = 0xFF;

            // Retrieve dimensions for the grid-window and pattern- 
            // brush controls.  
            GetClientRect(hwndGrid, &rctGrid);
            GetClientRect(hwndBrush, &rctBrush);

            // Determine the width and height of a single cell.  
            deltaX = (rctGrid.right - rctGrid.left) / 8;
            deltaY = (rctGrid.bottom - rctGrid.top) / 8;

            // Initialize the array of cell rectangles.  
            for (y = rctGrid.top, i = 0; y < rctGrid.bottom; y += deltaY)
            {
                for (x = rctGrid.left; x < (rctGrid.right - 8) && i < 64; x += deltaX, i++)
                {
                    rect[i].left = x; rect[i].top = y;
                    rect[i].right = x + deltaX;
                    rect[i].bottom = y + deltaY;
                }
            }
            return FALSE;

        case WM_PAINT:
            // Draw the grid.  
            hdc = GetDC(hwndGrid);

            for (i = rctGrid.left; i < rctGrid.right;
                i += (rctGrid.right - rctGrid.left) / 8)
            {
                MoveToEx(hdc, i, rctGrid.top, NULL);
                LineTo(hdc, i, rctGrid.bottom);
            }
            for (i = rctGrid.top; i < rctGrid.bottom;
                i += (rctGrid.bottom - rctGrid.top) / 8)
            {
                MoveToEx(hdc, rctGrid.left, i, NULL);
                LineTo(hdc, rctGrid.right, i);
            }
            ReleaseDC(hwndGrid, hdc);
            return FALSE;


        case WM_LBUTTONDOWN:
            // Store the mouse coordinates in a POINT structure.  
            ptlHit = MAKEPOINTS(lParam);

            // Create a rectangular region with dimensions and  
            // coordinates that correspond to those of the grid  
            // window.  
            hrgnCell = CreateRectRgn(rctGrid.left, rctGrid.top,
                rctGrid.right, rctGrid.bottom);

            // Retrieve a window DC for the grid window.  
            hdc = GetDC(hwndGrid);

            // Select the region into the DC.  
            SelectObject(hdc, hrgnCell);

            // Test for a button click in the grid-window rectangle.  
            if (PtInRegion(hrgnCell, ptlHit.x, ptlHit.y))
            {
                // A button click occurred in the grid-window  
                // rectangle; isolate the cell in which it occurred.  
                for (i = 0; i < 64; i++)
                {
                    DeleteObject(hrgnCell);

                    hrgnCell = CreateRectRgn(rect[i].left,
                        rect[i].top,
                        rect[i].right, rect[i].bottom);

                    if (PtInRegion(hrgnCell, ptlHit.x, ptlHit.y))
                    {
                        InvertRgn(hdc, hrgnCell);

                        // Set the appropriate brush bits.  

                        if (i % 8 == 0)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x80;
                        else if (i % 8 == 1)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x40;
                        else if (i % 8 == 2)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x20;
                        else if (i % 8 == 3)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x10;
                        else if (i % 8 == 4)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x08;
                        else if (i % 8 == 5)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x04;
                        else if (i % 8 == 6)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x02;
                        else if (i % 8 == 7)
                            bBrushBits[i / 8] = bBrushBits[i / 8] ^ 0x01;

                        // Exit the "for" loop after the bit is set.  

                        break;
                    } // end if  
                } // end for  
            } // end if  

            // Release the DC for the control.  
            ReleaseDC(hwndGrid, hdc);
            return TRUE;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDD_PAINTRECT:
                    hdc = GetDC(hwndBrush);

                    // Create a monochrome bitmap.  
                    hbm = CreateBitmap(8, 8, 1, 1,
                        (LPBYTE)bBrushBits);

                    // Select the custom brush into the DC.  
                    hbrush = CreatePatternBrush(hbm);

                    hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);

                    // Use the custom brush to fill the rectangle.  
                    Rectangle(hdc, rctBrush.left, rctBrush.top,
                        rctBrush.right, rctBrush.bottom);

                    // Clean up memory.  
                    SelectObject(hdc, hbrushOld);
                    DeleteObject(hbrush);
                    DeleteObject(hbm);

                    ReleaseDC(hwndBrush, hdc);
                    return TRUE;

                case IDD_OK:
                case IDD_CANCEL:
                    EndDialog(hdlg, TRUE);
                    return TRUE;

            } // end switch  
            break;
        default:
            return FALSE;
    }

    return false;
}

auto wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) -> int
{
	auto result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(101), nullptr, &BrushDlgProc, 0L);
	auto lastError = GetLastError();
	return 0;
}

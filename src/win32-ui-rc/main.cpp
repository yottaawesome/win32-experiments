// Based on https://learn.microsoft.com/en-us/windows/win32/gdi/using-brushes
// This sample appears to have bugs with it. The original sample specifies a
// GetStrLngth() and RetrieveWidth() functions aren't actually used anywhere.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "resource.h"

import std;

auto hwndGrid = HWND{};       // grid-window control  
auto hwndBrush = HWND{};      // pattern-brush control  
auto rctGrid = RECT{};        // grid-window rectangle  
auto rctBrush = RECT{};       // pattern-brush rectangle  
std::array<UINT, 8> bBrushBits = {};  // bitmap bits  
std::array<RECT, 64> rect = {};       // grid-cell array  
auto hbm = HBITMAP{};         // bitmap handle  

auto BrushDlgProc(
	HWND hdlg, 
	UINT message, 
	WPARAM wParam,
	LPARAM lParam
) -> INT_PTR
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			// Retrieve a window handle for the grid-window and  
			// pattern-brush controls.  
			hwndGrid = GetDlgItem(hdlg, IDD_GRID);
			hwndBrush = GetDlgItem(hdlg, IDD_RECT);

			// Initialize the array of bits that defines the  
			// custom brush pattern with the value 1 to produce a  
			// solid white brush. 
			for (auto& i : bBrushBits)
				i = 0xFF;

			// Retrieve dimensions for the grid-window and pattern- 
			// brush controls.  
			GetClientRect(hwndGrid, &rctGrid);
			GetClientRect(hwndBrush, &rctBrush);

			// Determine the width and height of a single cell.  
			int deltaX = (rctGrid.right - rctGrid.left) / static_cast<int>(bBrushBits.size());
			int deltaY = (rctGrid.bottom - rctGrid.top) / static_cast<int>(bBrushBits.size());

			// Initialize the array of cell rectangles.  
			for (int y = rctGrid.top, i = 0; y < rctGrid.bottom; y += deltaY)
			{
				for (int x = rctGrid.left; x < (rctGrid.right - static_cast<int>(bBrushBits.size())) && i < 64; x += deltaX, i++)
				{
					rect[i].left = x; rect[i].top = y;
					rect[i].right = x + deltaX;
					rect[i].bottom = y + deltaY;
				}
			}
			return FALSE;
		}

		case WM_PAINT:
		{
			// Draw the grid.  
			HDC hdc = GetDC(hwndGrid);

			for (int i = rctGrid.left; i < rctGrid.right; i += (rctGrid.right - rctGrid.left) / static_cast<int>(bBrushBits.size()))
			{
				MoveToEx(hdc, i, rctGrid.top, NULL);
				LineTo(hdc, i, rctGrid.bottom);
			}
			for (int i = rctGrid.top; i < rctGrid.bottom; i += (rctGrid.bottom - rctGrid.top) / static_cast<int>(bBrushBits.size()))
			{
				MoveToEx(hdc, rctGrid.left, i, NULL);
				LineTo(hdc, rctGrid.right, i);
			}
			ReleaseDC(hwndGrid, hdc);
			return FALSE;
		}

		case WM_LBUTTONDOWN:
		{
			// Store the mouse coordinates in a POINT structure.  
			POINTS ptlHit = MAKEPOINTS(lParam);

			// Create a rectangular region with dimensions and  
			// coordinates that correspond to those of the grid  
			// window.  
			HRGN hrgnCell = CreateRectRgn(rctGrid.left, rctGrid.top,
				rctGrid.right, rctGrid.bottom);

			// Retrieve a window DC for the grid window.  
			HDC hdc = GetDC(hwndGrid);

			// Select the region into the DC.  
			SelectObject(hdc, hrgnCell);

			// Test for a button click in the grid-window rectangle.  
			if (PtInRegion(hrgnCell, ptlHit.x, ptlHit.y))
			{
				// A button click occurred in the grid-window  
				// rectangle; isolate the cell in which it occurred.  
				for (int i = 0; i < 64; i++)
				{
					DeleteObject(hrgnCell);
					hrgnCell = CreateRectRgn(
						rect[i].left,
						rect[i].top,
						rect[i].right, 
						rect[i].bottom
					);

					if (PtInRegion(hrgnCell, ptlHit.x, ptlHit.y))
					{
						InvertRgn(hdc, hrgnCell);
						// Set the appropriate brush bits.  
						auto bitIndex = i / static_cast<int>(bBrushBits.size());
						auto bitModulo = i % static_cast<int>(bBrushBits.size());
						if (bitModulo == 0)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x80;
						else if (bitModulo == 1)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x40;
						else if (bitModulo == 2)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x20;
						else if (bitModulo == 3)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x10;
						else if (bitModulo == 4)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x08;
						else if (bitModulo == 5)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x04;
						else if (bitModulo == 6)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x02;
						else if (bitModulo == 7)
							bBrushBits[bitIndex] = bBrushBits[bitIndex] ^ 0x01;

						// Exit the "for" loop after the bit is set.  
						break;
					} // end if  
				} // end for  
			} // end if  

			// Release the DC for the control.  
			ReleaseDC(hwndGrid, hdc);
			return TRUE;
		}

		case WM_COMMAND:
			switch (wParam)
			{
				case IDD_PAINTRECT:
				{
					HDC hdc = GetDC(hwndBrush);
					// Create a monochrome bitmap.  
					hbm = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBrushBits.data());
					// Select the custom brush into the DC.  
					HBRUSH hbrush = CreatePatternBrush(hbm);
					HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);
					// Use the custom brush to fill the rectangle.  
					Rectangle(hdc, rctBrush.left, rctBrush.top, rctBrush.right, rctBrush.bottom);
					// Clean up memory.  
					SelectObject(hdc, hbrushOld);
					DeleteObject(hbrush);
					DeleteObject(hbm);

					ReleaseDC(hwndBrush, hdc);
					return TRUE;
				}

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
	auto result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(CustBrush), nullptr, &BrushDlgProc, 0L);
	auto lastError = GetLastError();
	return 0;
}

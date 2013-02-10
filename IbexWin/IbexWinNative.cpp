// IbexWinNative.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "IbexWinNative.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


#include <stdio.h>

#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

typedef unsigned long Window;
typedef unsigned long GLXContext;

#else
#ifdef _WIN32

#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "ibex_win_utils.h"

typedef unsigned long Window;
typedef unsigned long GLXContext;

#else

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

#endif
#endif

#include "math_3d.h"

#include "ibex.h"

GLuint VBO;

void Keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 27:             // ESCAPE key
	  exit (0);
	  break;
  case 'w':
	  walkForward = 1;
	  break;
  case 'a':
  case 'q':
	  strafeRight = -1;
	  break;
  case 's':
	  walkForward = -1;
	  break;
  case 'd':
  case 'e':
	  strafeRight = 1;
	  break;
  case 'g':
	  showGround = !showGround;
	  break;
  case 'r':
	  resetPosition = 1;
	  break;
  }
}
void KeyboardUp(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 27:             // ESCAPE key
	  exit (0);
	  break;
  case 'w':
	  walkForward = 0;
	  break;
  case 'a':
  case 'q':
	  strafeRight = 0;
	  break;
  case 's':
	  walkForward = 0;
	  break;
  case 'd':
  case 'e':
	  strafeRight = 0;
	  break;
  }
}
//- (void)keyDown:(NSEvent *)theEvent {
//    switch(theEvent.keyCode) {
//        case kVK_ANSI_W:
//            walkForward = 1;
//            break;
//        case kVK_ANSI_S:
//            walkForward = -1;
//            break;
//        case kVK_ANSI_A:
//            strafeRight = -1;
//            break;
//        case kVK_ANSI_D:
//            strafeRight = 1;
//            break;
//        case kVK_Space:
//            break;
//    }
//}
//- (void)keyUp:(NSEvent *)theEvent {
//    switch(theEvent.keyCode) {
//        case kVK_ANSI_B:
//            barrelDistort = !barrelDistort;
//            break;
//        case kVK_ANSI_G:
//            showGround = !showGround;
//            break;
//        case kVK_ANSI_R:
//            resetPosition = 1;
//            break;
//        case kVK_ANSI_W:
//            walkForward = 0;
//            break;
//        case kVK_ANSI_S:
//            walkForward = 0;
//            break;
//        case kVK_ANSI_A:
//            strafeRight = 0;
//            break;
//        case kVK_ANSI_D:
//            strafeRight = 0;
//            break;
//        case kVK_Space:
//            break;
//    }
//}

void MouseMoved(int x, int y) {
    relativeMouseX = x-500;
    relativeMouseY = y-500;
    
//    NSLog(@"%f, %f", relativeMouseX, relativeMouseY);
}

void errhandler(const char *e, HWND w) {
}
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        errhandler("GetObject", hwnd); 

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

     if (cClrBits < 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (1<< cClrBits)); 

     // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
     pbmi->bmiHeader.biClrImportant = 0; 
     return pbmi; 
 } 

// -------------------------------------------------------------------------
// When tested - the printout cuts off the right side of the screen.
// It is not much, but still I cannot find the explanation for it.
// All values for printer are retrieved using API - all done by the book,
// but looks like the assumption that left margin for printer is
// the same as right margin is incorrect. I assumed that they are the same and
// the expression for horizontal printable area is:
//	int printedWidth = xPrnPixelsPerPage - xPrnPixelsMargin * 2;
// Easy fix will be to use 3 margins(!) instead of 2:
//	int printedWidth = xPrnPixelsPerPage - xPrnPixelsMargin * 3;
// Or play with the 'xPrnPixelsMargin' value itself (decrease it a little).
// Still, no explanation bothers me, but function works fine otherwise.
// -------------------------------------------------------------------------
void PrintScreen (HDC hDest)
	{
	HDC hScreenDC;

	if ((hScreenDC = GetDC (NULL)) != NULL) {
		HDC hMemDC;

		if ((hMemDC = CreateCompatibleDC (hScreenDC)) != NULL) {
			HBITMAP hFullScreenBitmap;
			int iScreenWidth = GetSystemMetrics (SM_CXSCREEN);
			int iScreenHeight = GetSystemMetrics (SM_CYSCREEN);
			
			if ((hFullScreenBitmap = CreateCompatibleBitmap (hScreenDC, iScreenWidth, iScreenHeight)) != NULL) {
				HBITMAP hDefaultBitmapDest = (HBITMAP) SelectObject (hDest, hFullScreenBitmap);
				BitBlt (hDest, 0, 0, iScreenWidth, iScreenHeight, hScreenDC, 0, 0, SRCCOPY|CAPTUREBLT);
				return;

				HBITMAP hDefaultBitmap = (HBITMAP) SelectObject (hMemDC, hFullScreenBitmap);

				// From screen to memory:
				BitBlt (hMemDC, 0, 0, iScreenWidth, iScreenHeight, hScreenDC, 0, 0, SRCCOPY);

				
				
						HWND desktop = GetDesktopWindow();
		//CreateBMPFile(desktop, L"blah2.bmp", CreateBitmapInfoStruct(desktop, hFullScreenBitmap), hFullScreenBitmap, hMemDC);
		//exit(0);

				SelectObject (hMemDC, hDefaultBitmap);
				SelectObject (hMemDC, hFullScreenBitmap);

				SelectObject (hMemDC, hDefaultBitmap);
				DeleteObject (hDefaultBitmap);
				DeleteObject (hFullScreenBitmap);
				}
			DeleteDC (hMemDC);
			}
		ReleaseDC (NULL, hScreenDC);
		}
	}

int CaptureAnImage(HWND hWnd)
{
    HDC hdcScreen;
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcScreen = GetDC(NULL);
    hdcWindow = GetDC(hWnd);

    // Create a compatible DC which is used in a BitBlt from the window DC
    hdcMemDC = CreateCompatibleDC(hdcWindow); 

    if(!hdcMemDC)
    {
        MessageBox(hWnd, L"CreateCompatibleDC has failed",L"Failed", MB_OK);
        goto done;
    }

    // Get the client area for size calculation
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    //This is the best stretch mode
    //SetStretchBltMode(hdcWindow,HALFTONE);

    //The source DC is the entire screen and the destination DC is the current window (HWND)
    //if(!StretchBlt(hdcWindow, 
    //           0,0, 
    //           width, height,//rcClient.right, rcClient.bottom, 
    //           hdcScreen, 
    //           0,0,
    //           GetSystemMetrics (SM_CXSCREEN),
    //           GetSystemMetrics (SM_CYSCREEN),
    //           SRCCOPY))
    //{
    //    MessageBox(hWnd, L"StretchBlt has failed",L"Failed", MB_OK);
    //    goto done;
    //}
    
    // Create a compatible bitmap from the Window DC
    hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);
    
    if(!hbmScreen)
    {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed",L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC,hbmScreen);
    
    // Bit block transfer into our compatible memory DC.
    if(!BitBlt(hdcMemDC, 
               0,0, 
               rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, 
               hdcWindow, 
               0,0,
               SRCCOPY))
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP
    GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);
     
    BITMAPFILEHEADER   bmfHeader;    
    BITMAPINFOHEADER   bi;
     
    bi.biSize = sizeof(BITMAPINFOHEADER);    
    bi.biWidth = bmpScreen.bmWidth;    
    bi.biHeight = bmpScreen.bmHeight;  
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
    char *lpbitmap = (char *)GlobalLock(hDIB);    

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

		glBindTexture(GL_TEXTURE_2D, desktopTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bmpScreen.bmWidth, bmpScreen.bmHeight, 0,
               GL_BGRA, GL_UNSIGNED_BYTE, lpbitmap);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glBindTexture(GL_TEXTURE_2D, 0);

    //Unlock and Free the DIB from the heap
    GlobalUnlock(hDIB);    
    GlobalFree(hDIB);
       
    //Clean up
done2:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL,hdcScreen);
    ReleaseDC(hWnd,hdcWindow);

return 0;

    // A file is created, this is where we will save the screen capture.
    HANDLE hFile = CreateFile(L"captureqwsx.bmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);   
    
    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
 
    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER); 
    
    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB; 
    
    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM   
 
    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
    
    //Unlock and Free the DIB from the heap
    GlobalUnlock(hDIB);    
    GlobalFree(hDIB);

    //Close the handle for the file that was created
    CloseHandle(hFile);
       
    //Clean up
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL,hdcScreen);
    ReleaseDC(hWnd,hdcWindow);

    return 0;
}


void getScreenshot() {
	static bool ran = false;
	//if(ran) return;
	ran = true;

	// get desktop window handle 	

	//HWND GLhwnd = GetActiveWindow();
	//ShowWindow(GLhwnd, SW_HIDE);
HWND hwnd = GetDesktopWindow();
CaptureAnImage(hwnd);
//ShowWindow(GLhwnd, SW_SHOW);

return;

// get its size

RECT rc;
GetWindowRect(hwnd, &rc);
int width  = rc.right;    // rc.left = 0!

int height = rc.bottom;   // rc.top  = 0!

	
// this is the image class that I use for different

// purpose (you can use a DWORD buffer of size w x h)

//ImageRGBA.Create(width, height);
GLbyte *data = new GLbyte[width*height*3];

// GetDC, GetDCEx do not work...this works!	

HDC  hdc  = GetWindowDC(hwnd);

// Redraw everything	

//RedrawWindow(NULL, NULL, NULL, RDW_UPDATENOW);

// grab every pixel 

for(int x=0; x<width; ++x)
{
  for(int y=0; y<height; ++y)
  {
    // windows uses 0x00bbggrr

    COLORREF wincolor = GetPixel(hdc, x, y);
    
    BYTE r = (wincolor>>0)&0xff;
    BYTE g = (wincolor>>8)&0xff;
    BYTE b = (wincolor>>16)&0xff;
    
    // convert to my ColorModel (I use a RGBA model

    // for OpenGL)(now it's late for me and I dont remember

    // if the conversion is necessary...however the inline

    // function and various #defines solve the problem for me)

    //AC_COLORARGB mycolor = acColorARGB(r, g, b, 0);
    
    //for example  buffer[x + y*width] = yourcolor(r,g,b,a)

    //ImageRGBA.SetPixel(x, y, mycolor);
	data[x+y*width] = r;//r<<16|g<<8|b;
  }
}

// Release the device context	

ReleaseDC(hwnd, hdc);

//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, 0, data);
glBindTexture(GL_TEXTURE_2D, desktopTexture);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, data);
glBindTexture(GL_TEXTURE_2D, 0);
delete [] data;

//	char *data;
//	int GetDIBits(
//  hScreenDC,
//  hDefaultBitmap,
//  0,
//  0,
//  data,//_Out_    LPVOID lpvBits,
//  CreateBitmapInfoStruct(hScreenDC, hDefaultBitmap),//_Inout_  LPBITMAPINFO lpbi,
//  DIB_RGB_COLORS//_In_     UINT uUsage
//);
}

Ibex *ibex = 0;
static void RenderSceneCB()
{
	static double timeprev = glutGet(GLUT_ELAPSED_TIME);
	double time = glutGet(GLUT_ELAPSED_TIME);
	double timeDiff = (time - timeprev)/1000.0;
	timeprev = time;

	glutWarpPointer(500, 500);

	// Add your drawing codes here
    if(ibex == 0) {
		char *argv[] = {""};
        ibex = new Ibex(0,0);
    }
    
	getScreenshot();
    cursorPosX = 50;//cursorPos.x;
    cursorPosY = 50;//cursorPos.y;
    
    ibex->render(timeDiff);

	glutSwapBuffers();
	glutPostRedisplay();
}


static void InitializeGlutCallbacks()
{
	glutWarpPointer(500, 500);

    glutDisplayFunc(RenderSceneCB);
	glutKeyboardFunc (Keyboard);
	glutKeyboardUpFunc (KeyboardUp);
	glutPassiveMotionFunc (MouseMoved);
}

static void CreateVertexBuffer()
{
    Vector3f Vertices[3];
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
    Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
    Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);

 	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	width = 1280;
	height = 800;
	physicalWidth = width;
	physicalHeight = height;

	controlDesktop = 0;

	size_t len = 0;
	wchar_t cwd[1024];
	//mbstowcs_s
	GetCurrentDirectory(1023, cwd);
	wcstombs_s(&len, mResourcePath, cwd, 1023);

	//physicalHeight = 900;
	//physicalWidth = 1440;
	//width = 1440;
	//height = 900;

	int argc = 0;
	char **argv = 0;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(width, height);////1440, 900);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Ibex");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //CreateVertexBuffer();

    glutMainLoop();

	return 0;
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IBEXWINNATIVE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IBEXWINNATIVE));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IBEXWINNATIVE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_IBEXWINNATIVE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

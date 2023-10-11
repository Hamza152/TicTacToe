#include <windows.h>
#include <stdint.h>
#include <windowsx.h>
#include <math.h>
#include <cstdio>

#define internal static 
#define local_persist static 
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


global_variable uint32 Black = 0;
global_variable uint32 White = 0xffffffff;

global_variable char RecentGameFile[] = "../images/LastGameSave.txt";
global_variable char LittleCercleFileName[] = "../images/LittleCercleImg.bmp"; 
global_variable char BigCercleFileName[] = "../images/BigCercleImg.bmp";
global_variable char LittleXFileName[] = "../images/LittleXImg.bmp";
global_variable char BigXFileName[] = "../images/BigXImg.bmp";
global_variable char XWinnerFileName[] = "../images/XWinnerImg.bmp";
global_variable char CercleWinnerFileName[] = "../images/CercleWinnerImg.bmp";
global_variable char WelcomeBGNDFileName[] = "../images/Welcome.bmp";
global_variable char PlayButtonFileName[] = "../images/PlayButton.bmp";
global_variable char RePlayButtonFileName[] = "../images/RePlayButton.bmp";
global_variable char ContinueButtonFileName[] = "../images/ContinueButton.bmp";

global_variable uint32* LittleCercleImage = NULL;
global_variable uint32* BigCercleImage = NULL;
global_variable uint32* LittleXImage = NULL;
global_variable uint32* BigXImage = NULL;
global_variable uint32* WelcomePage = NULL;
global_variable uint32* PlayButtonImage = NULL;
global_variable uint32* RePlayButtonImage = NULL;
global_variable uint32* ContinueButtonImage = NULL;
global_variable uint32* XWinnerImage = NULL;
global_variable uint32* CercleWinnerImage = NULL;

global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

global_variable bool Running;
global_variable int AnimationsCount = 0;
global_variable bool XRendered = 0;
global_variable bool Win = false;
global_variable bool Draw = false;
global_variable bool NewGame = false;
global_variable bool ContinueOldGame = false;
global_variable bool OldGameExist = false;
global_variable bool RenderingWin = false;

global_variable double TimeCounter = 0;
global_variable bool WinningAnim = false;

LARGE_INTEGER PerformanceFrequencyL;

struct Box{
    int X;
    int Y;
    int Symbol;
    bool Paint = 0;
    bool WinningBox = false;
    double BoxTimeCounter = 0;
    bool FirstTimeDrawing = true;
};

global_variable Box Boxes[9]= {  {1,1,1},
                                 {1,2,1},
                                 {1,3,1},
                                 {2,1,1},
                                 {2,2,1},
                                 {2,3,1},
                                 {3,1,1},
                                 {3,2,1},
                                 {3,3,1}
                             };


struct Point{
    int X = -1;
    int Y = -1;
};

struct MyRectangle {
    Point TopLeft = { 0,0 };
    int Width = 0;
    int Height = 0;
};

MyRectangle PlayButton;
MyRectangle RePlayButton;
MyRectangle ContinueButton;
MyRectangle LittleXSymbol;
MyRectangle BigXSymbol;
MyRectangle LittleCercleSymbol;
MyRectangle BigCercleSymbol;

struct MouseTrackingVars {
    bool LButtonDown = 0;
    bool LButtonUp = 0;
    Point DownCursorPt;
    Point UpCursorPt;
};

global_variable MouseTrackingVars MyMouse;
global_variable const MouseTrackingVars  EmptyMouse = {};

internal void
GetBMPPixelArray(uint32** Image, char* FileName, int *ImageWidth = NULL, int *ImageHeight = NULL, int *FileBytesPerPixel = NULL)
{
    HANDLE FileHnd = CreateFileA(FileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    uint8 DataBuffer[4] = {};
    LPVOID VPDataBuffer = (void*)DataBuffer;
    DWORD NbBytesToRead = 4;
    DWORD BytesRead = 0;
    DWORD FilePointer = 0;

    uint32 BitmapDataStartingPointer;
    FilePointer = SetFilePointer(FileHnd, 10, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    BitmapDataStartingPointer = (DataBuffer[3] << 24 | DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);

    FilePointer = SetFilePointer(FileHnd, 18, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    if(ImageWidth)
        *ImageWidth = (DataBuffer[3] << 24 | DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);

    FilePointer = SetFilePointer(FileHnd, 22, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    if(ImageHeight)
        *ImageHeight = (DataBuffer[3] << 24 | DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);

    FilePointer = SetFilePointer(FileHnd, 28, NULL, 0);
    ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
    if (FileBytesPerPixel) {
        *FileBytesPerPixel = (DataBuffer[1] << 8 | DataBuffer[0]);
        *FileBytesPerPixel /= 8;
    }
    int NumPixels;
    if (ImageWidth)
        NumPixels = (*ImageWidth) * (*ImageHeight);
    else
        NumPixels = BitmapWidth * BitmapHeight;
    
    uint32* MyImage = new uint32[NumPixels];

    if (FileBytesPerPixel)
        NbBytesToRead = *FileBytesPerPixel;
    else
        NbBytesToRead = 3;

    FilePointer = SetFilePointer(FileHnd, BitmapDataStartingPointer, NULL, 0);
    for (int i = 0; i < NumPixels; i++) {
        ReadFile(FileHnd, VPDataBuffer, NbBytesToRead, &BytesRead, NULL);
        MyImage[i] = (DataBuffer[2] << 16 | DataBuffer[1] << 8 | DataBuffer[0]);
    }
    *Image = MyImage;
    CloseHandle(FileHnd);
}

internal void
LoadBitmapFiles() {
    PlayButton.TopLeft = { BitmapWidth / 4, BitmapHeight - (2 * BitmapHeight) / 8 };
    RePlayButton.TopLeft = { BitmapWidth / 4, BitmapHeight - (2 * BitmapHeight) / 8 };
    ContinueButton.TopLeft = { BitmapWidth / 4, BitmapHeight - (4 * BitmapHeight) / 8 };
    GetBMPPixelArray(&PlayButtonImage, PlayButtonFileName, &PlayButton.Width, &PlayButton.Height);
    GetBMPPixelArray(&RePlayButtonImage, RePlayButtonFileName, &RePlayButton.Width, &RePlayButton.Height);
    GetBMPPixelArray(&ContinueButtonImage, ContinueButtonFileName, &ContinueButton.Width, &ContinueButton.Height);

    GetBMPPixelArray(&WelcomePage, WelcomeBGNDFileName);
    GetBMPPixelArray(&XWinnerImage, XWinnerFileName);
    GetBMPPixelArray(&CercleWinnerImage, CercleWinnerFileName);

    GetBMPPixelArray(&LittleXImage, LittleXFileName, &LittleXSymbol.Width, &LittleXSymbol.Height);
    GetBMPPixelArray(&BigXImage, BigXFileName, &BigXSymbol.Width, &BigXSymbol.Height);
    GetBMPPixelArray(&LittleCercleImage, LittleCercleFileName, &LittleCercleSymbol.Width, &LittleCercleSymbol.Height);
    GetBMPPixelArray(&BigCercleImage, BigCercleFileName, &BigCercleSymbol.Width, &BigCercleSymbol.Height);
}

internal void
InsertImage(uint32* Img, MyRectangle ImgInfo, int BoxIndex = -1)
{
    uint32* Pixel = (uint32*)BitmapMemory;
    int Pitch = BitmapWidth;

    int LinesToDraw = 0;

    if (BoxIndex != -1)
    {
        if (Boxes[BoxIndex].FirstTimeDrawing == true)
        {
            Boxes[BoxIndex].BoxTimeCounter = 0;
            Boxes[BoxIndex].FirstTimeDrawing = false;
        }
        LinesToDraw = (Boxes[BoxIndex].BoxTimeCounter * LittleCercleSymbol.Height) / 0.3;
        if (LinesToDraw > LittleCercleSymbol.Height)
            LinesToDraw = LittleCercleSymbol.Height;
        LinesToDraw -= ImgInfo.Height;
    }

    int i = 0;
    for (int Y = ImgInfo.TopLeft.Y + ImgInfo.Height - 1; Y >= ImgInfo.TopLeft.Y - LinesToDraw; Y--)
    {
        for (int X = ImgInfo.TopLeft.X; X < ImgInfo.TopLeft.X + ImgInfo.Width; X++)
        {
            Pixel[Y * Pitch + X] = Img[i];
            i++;
        }
    }
}

internal Box
GetBox(Point P) {
    int i = float(3 * P.X) / BitmapWidth + 1;
    int j = float(3 * P.Y) / BitmapHeight + 1;
    return { i, j };
}

internal Point
GetBoxWindowTopLeft(int BoxIndex)
{

    int BoxWidth = (BitmapWidth - 28) / 3;
    int BoxHeight = (BitmapHeight - 28)/ 3;
    int BoxReferenceX, BoxReferenceY;
    if (Boxes[BoxIndex].X == 0)
        BoxReferenceX = 7;
    else
        BoxReferenceX = (Boxes[BoxIndex].X - 1) * (int(BitmapWidth / 3)) + 4;
    if (Boxes[BoxIndex].Y == 0)
        BoxReferenceY = 7;
    else
        BoxReferenceY = (Boxes[BoxIndex].Y - 1) * (int(BitmapHeight / 3)) + 4;
    return { BoxReferenceX + int(BoxWidth / 8), BoxReferenceY + int(BoxHeight / 8) };
}

internal void
RenderXOGrid()
{
    int Width = BitmapWidth;
    int Height = BitmapHeight;

    int Pitch = Width * BytesPerPixel;
    uint8* Row = (uint8*)BitmapMemory;
    for (int Y = 0;
        Y < BitmapHeight;
        ++Y)
    {
        uint32* Pixel = (uint32*)Row;
        if ((Y < 7) || (Y >= BitmapHeight - 7) || (Y >= int(BitmapHeight / 3) - 3 && Y <= int(BitmapHeight / 3) + 3) || (Y >= 2 * int(BitmapHeight / 3) - 3 && Y <= 2 * int(BitmapHeight / 3) + 3)) {
            for (int X = 0; X < BitmapWidth; ++X) {
                uint8 Red = 100;
                uint8 Blue = 100;
                uint8 Green = 100;
                *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
            }
        }
        else {
            for (int X = 0;
                X < BitmapWidth;
                ++X)
            {
                if ((X < 7) || (X >= BitmapWidth - 7) || (X >= int(BitmapWidth / 3) - 3 && X <= int(BitmapWidth / 3) + 3) || (X >= 2 * int(BitmapWidth / 3) - 3 && X <= 2 * int(BitmapWidth / 3) + 3)) {
                    *Pixel++ = Black;
                }
                else {
                    *Pixel++ = White;
                }

            }
        }

        Row += Pitch;
    }
}

internal void
renderSymbol(int BoxIndex)
{
    int BoxWidth = (BitmapWidth - 28) / 3;
    int BoxHeight = (BitmapHeight - 28)/ 3;
    int SymbolWidth = BoxWidth - 2*int(BoxWidth/8);
    int SymbolHeight = BoxHeight - 2*int(BoxHeight/8);
    
    Point TLSymbol = GetBoxWindowTopLeft(BoxIndex);
    Point TRSymbol = {TLSymbol.X + SymbolWidth, TLSymbol.Y};
    Point BLSymbol = { TLSymbol.X,  TLSymbol.Y + SymbolHeight};
    Point BRSymbol = { TLSymbol.X + SymbolWidth, TLSymbol.Y + SymbolHeight};
    Point SymbolCenter = { TLSymbol.X + int(SymbolWidth / 2), TLSymbol.Y + int(SymbolHeight / 2) };

    if (!WinningAnim)
    {
        if (Boxes[BoxIndex].Symbol == 2)
        {
            InsertImage(LittleXImage, { TLSymbol, LittleXSymbol.Width, LittleXSymbol.Height }, BoxIndex);
        }
        if (Boxes[BoxIndex].Symbol == 3) {
            InsertImage(LittleCercleImage, { TLSymbol, LittleCercleSymbol.Width , LittleCercleSymbol.Height }, BoxIndex);
        }
    }
    else {
        if (Boxes[BoxIndex].Symbol == 2)
        {
            if (!Boxes[BoxIndex].WinningBox)
                InsertImage(LittleXImage, { TLSymbol, LittleXSymbol.Width, LittleXSymbol.Height}, BoxIndex);
            else
                InsertImage(BigXImage, { TLSymbol, BigXSymbol.Width, BigXSymbol.Height});
        }
        if (Boxes[BoxIndex].Symbol == 3) {
            if (!Boxes[BoxIndex].WinningBox)
                InsertImage(LittleCercleImage, { TLSymbol, LittleCercleSymbol.Width, LittleCercleSymbol.Height }, BoxIndex);
            else
                InsertImage(BigCercleImage, { TLSymbol, BigCercleSymbol.Width, BigCercleSymbol.Height});
        }
    }
}


internal void
RenderSymbols() {
    for(int BoxIndex = 0; BoxIndex < 9; BoxIndex++){
        if (Boxes[BoxIndex].Paint == 1) 
            renderSymbol(BoxIndex);
    }

}

internal void
RenderDraw()
{
    int Pitch = BitmapWidth;
    int Red = 255;
    int Blue = 0;
    int Green = 0;
    uint32* Pixel = (uint32*)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y)
    {
        for (int X = 0; X < BitmapWidth; X++)
            Pixel[Y * Pitch + X] = ((Red << 16) | (Green << 8) | Blue);
    }
}

internal void
RenderGame() {
    RenderXOGrid();
    RenderSymbols();
}



internal void
RenderWinningPage() {
    if(Win)
    {
        int i = 0;
        for (i = 0; i < 9 && !Boxes[i].WinningBox; i++);
        if (Boxes[i].Symbol == 2)
            InsertImage(XWinnerImage, {{0,0},BitmapWidth, BitmapHeight});
        else
            InsertImage(CercleWinnerImage, {{0,0},BitmapWidth, BitmapHeight});
    }
    if(Draw)
        
    InsertImage(RePlayButtonImage, RePlayButton);
}

internal void
RenderWin()
{
    if (AnimationsCount < 4)
    {
        if (TimeCounter < 0.3)
            WinningAnim = false;
        else
        {

            WinningAnim = true;
            if (TimeCounter > 0.6)
            {
                AnimationsCount++;
                TimeCounter = 0;
            }
        }
        RenderGame();
    }
    else
        RenderWinningPage();
}

internal void
CheckGameState()
{
    // Lines Check
    for (int i = 0; i < 9; i += 3)
        if ((Boxes[i].Symbol == 2 && Boxes[i + 1].Symbol == 2 && Boxes[i + 2].Symbol == 2) || (Boxes[i].Symbol == 3 && Boxes[i + 1].Symbol == 3 && Boxes[i + 2].Symbol == 3))
        {
            Win = true;
            Boxes[i].WinningBox = 1;
            Boxes[i+1].WinningBox = 1;
            Boxes[i+2].WinningBox = 1;
        }

    //Columns Check
    for(int i = 0; i <3; i++)
        if ((Boxes[i].Symbol == 2 && Boxes[i + 3].Symbol == 2 && Boxes[i + 6].Symbol == 2) || (Boxes[i].Symbol == 3 && Boxes[i + 3].Symbol == 3 && Boxes[i + 6].Symbol == 3))
        {
            Win = true;
            Boxes[i].WinningBox = 1;
            Boxes[i + 3].WinningBox = 1;
            Boxes[i + 6].WinningBox = 1;
        }
    //Diagonals Check
    if ((Boxes[0].Symbol == 2 && Boxes[4].Symbol == 2 && Boxes[8].Symbol == 2) || (Boxes[0].Symbol == 3 && Boxes[4].Symbol == 3 && Boxes[8].Symbol == 3))
    {
        Win = true;
        Boxes[0].WinningBox = 1;
        Boxes[4].WinningBox = 1;
        Boxes[8].WinningBox = 1;

    }
    if ((Boxes[2].Symbol == 2 && Boxes[4].Symbol == 2 && Boxes[6].Symbol == 2) || (Boxes[2].Symbol == 3 && Boxes[4].Symbol == 3 && Boxes[6].Symbol == 3))
    {
        Win = true;
        Boxes[2].WinningBox = 1;
        Boxes[4].WinningBox = 1;
        Boxes[6].WinningBox = 1;
    }
    int i =0;
    for(i = 0; i < 9; i++)
        if(Boxes[i].Paint == 0)
            break;
    if (i == 9)
        Draw = true;

}

internal void
CheckNewGame()
{
    if (MyMouse.LButtonUp == 1 && MyMouse.LButtonDown == 1) {
        if (MyMouse.DownCursorPt.X < PlayButton.TopLeft.X + PlayButton.Width && MyMouse.DownCursorPt.X > PlayButton.TopLeft.X && 
            MyMouse.DownCursorPt.Y < PlayButton.TopLeft.Y + PlayButton.Height && MyMouse.DownCursorPt.Y > PlayButton.TopLeft.Y)
        {
            NewGame = true;
            MyMouse = EmptyMouse;
        }
    }
}

internal void
CheckContinueGame()
{
    if (MyMouse.LButtonUp == 1 && MyMouse.LButtonDown == 1) {
        if (MyMouse.DownCursorPt.X < ContinueButton.TopLeft.X + ContinueButton.Width && MyMouse.DownCursorPt.X > ContinueButton.TopLeft.X &&
            MyMouse.DownCursorPt.Y < ContinueButton.TopLeft.Y + ContinueButton.Height && MyMouse.DownCursorPt.Y > ContinueButton.TopLeft.Y)
        {
            ContinueOldGame = true;
            MyMouse = EmptyMouse;
        }
    }
}

internal void
InitialiseGameParams()
{
    Win = false;
    Draw = false;
    TimeCounter = 0;
    XRendered = false;
    MyMouse = EmptyMouse;
    RenderingWin = false;
    AnimationsCount = 0;
    for (int i = 0; i < 9; i++)
    {
        Boxes[i].Paint = 0;
        Boxes[i].FirstTimeDrawing = true;
        Boxes[i].Symbol = 1;
        Boxes[i].WinningBox = false;
    }
}
internal void
CheckGameReplay()
{
    if (MyMouse.LButtonUp == 1 && MyMouse.LButtonDown == 1) {
        if (MyMouse.DownCursorPt.X < RePlayButton.TopLeft.X + RePlayButton.Width && MyMouse.DownCursorPt.X > RePlayButton.TopLeft.X &&
            MyMouse.DownCursorPt.Y < RePlayButton.TopLeft.Y + RePlayButton.Height && MyMouse.DownCursorPt.Y > RePlayButton.TopLeft.Y)
        {
            NewGame = true;
            InitialiseGameParams();
        }
    }
}


internal void 
ApplyGameChanges() {
    if (MyMouse.LButtonUp == 1 && MyMouse.LButtonDown == 1) {
        Box DownBox = GetBox(MyMouse.DownCursorPt);
        Box UpBox = GetBox(MyMouse.UpCursorPt);
        // Detect if clicked in one box
        if (DownBox.X == UpBox.X && DownBox.Y == UpBox.Y && DownBox.X != 0){
            int BoxIndex = DownBox.Y - 1 + 3 * (DownBox.X - 1);
            if(Boxes[BoxIndex].Paint == 0){
                Boxes[BoxIndex].Paint = 1;
                if (!XRendered)
                {
                    Boxes[BoxIndex].Symbol = 2;
                    XRendered = true;
                }
                else
                {
                    Boxes[BoxIndex].Symbol = 3;
                    XRendered = false;
                }
            }
        }
        MyMouse = EmptyMouse;
    }
}


internal void 
SaveGameState() {
    HANDLE FileHnd = CreateFileA(RecentGameFile,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    LPVOID VXRendered = (void*)(&XRendered);
    LPVOID VBoxes = (void*)Boxes;

    DWORD NumBytesWritten;
    WriteFile(
        FileHnd,
        VXRendered,
        sizeof(XRendered),
        &NumBytesWritten,
        NULL
    );

    WriteFile(
        FileHnd,
        VBoxes,
        sizeof(Boxes),
        &NumBytesWritten,
        NULL
    );
    CloseHandle(FileHnd);
}

internal void
CheckRecentGameExistence() {
    HANDLE FileHnd = CreateFileA(RecentGameFile,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (FileHnd != INVALID_HANDLE_VALUE) {

        LPVOID VXRendered = (void*)(&XRendered);
        LPVOID VBoxes = (void*)Boxes;

        DWORD BytesRead;
        ReadFile(FileHnd,
            VXRendered,
            sizeof(XRendered),
            &BytesRead,
            NULL);
        if(BytesRead != 0)
        {
            OldGameExist = true;
            ReadFile(FileHnd,
                VBoxes,
                sizeof(Boxes),
                &BytesRead,
                NULL
            );
        }
    }
    CloseHandle(FileHnd);
}

internal void 
EraseSavedGames() {
    HANDLE FileHnd = CreateFileA(RecentGameFile,
        GENERIC_WRITE,
        0,
        NULL,
        TRUNCATE_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    CloseHandle(FileHnd);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT* ClientRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(DeviceContext,
        0, 0, BitmapWidth, BitmapHeight,
        0, 0, WindowWidth, WindowHeight,
        BitmapMemory,
        &BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{ 
    if(BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
     
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

}


LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;

    switch(Message)
    {
        case WM_LBUTTONDOWN:
        {
            int X_cursor = GET_X_LPARAM(LParam);
            int Y_cursor = GET_Y_LPARAM(LParam);
            if (MyMouse.LButtonUp == 1)
                MyMouse.LButtonUp = 0;
            MyMouse.LButtonDown = 1;
            MyMouse.DownCursorPt.X = X_cursor;
            MyMouse.DownCursorPt.Y = Y_cursor;
        }break;

        case WM_LBUTTONUP:
        {
            int X_cursor = GET_X_LPARAM(LParam);
            int Y_cursor = GET_Y_LPARAM(LParam);
            MyMouse.LButtonUp = 1;
            MyMouse.UpCursorPt.X = X_cursor;
            MyMouse.UpCursorPt.Y = Y_cursor;
        }break;

        case WM_MOUSELEAVE:
        {
            MyMouse = EmptyMouse;
        }break;

        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;

        case WM_CLOSE:
        {
            Running = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            Running = false;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);

            Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

 
int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    QueryPerformanceFrequency(&PerformanceFrequencyL);
    int64 PerformanceFrequency = PerformanceFrequencyL.QuadPart;
    WNDCLASS WindowClass = {};
     
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
//    WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                500,
                500,
                0,
                0,
                Instance,
                0);
        if(Window)
        {
            Running = true;
            LoadBitmapFiles();
            CheckRecentGameExistence();
            while(Running)
            {
                LARGE_INTEGER BeginTime, EndTime;
                QueryPerformanceCounter(&BeginTime);
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;

                if (!(NewGame || ContinueOldGame))
                {
                    //RenderWelcomePage
                    InsertImage(WelcomePage, { {0,0},BitmapWidth, BitmapHeight });
                    InsertImage(PlayButtonImage, PlayButton);
                    // Check for old game
                    if (OldGameExist)
                        InsertImage(ContinueButtonImage, ContinueButton);

                    // handle user input
                    CheckNewGame();
                    CheckContinueGame();
                    if (NewGame)
                        InitialiseGameParams();
                }
                else {
                    // Continue the game
                    if (!(Win || Draw))
                    {
                        ApplyGameChanges();
                        RenderGame();
                        CheckGameState();
                    }
                    // Check in or draw status
                    else {
                        if (Win)
                        {
                            if (!RenderingWin)
                            {
                                TimeCounter = -0.3;
                                RenderingWin = true;
                            }
                            RenderWin();
                            if (TimeCounter > 0.6)
                                CheckGameReplay();
                            else
                                MyMouse = EmptyMouse;
                        }
                        else
                            RenderDraw();
                        EraseSavedGames();
                    }
                }
                Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                QueryPerformanceCounter(&EndTime);
                TimeCounter += double(EndTime.QuadPart - BeginTime.QuadPart)/PerformanceFrequency;
                for (int i = 0; i < 9; i++)
                    Boxes[i].BoxTimeCounter += double(EndTime.QuadPart - BeginTime.QuadPart) / PerformanceFrequency;
            }
            if (!(Win || Draw))
                SaveGameState();
        }
        else
        { 
        }
    }
    else
    { 
    }
    
    return(0);
}


#pragma once

#include "utilities.h"

struct Window {
    void* window;
    bool closeRequested;
};

static bool Key_Inputs[256];

#ifdef _WIN64
#include <windows.h>
#include <stdio.h>

static const u8 KEY_0 = 0x30;
static const u8 KEY_1 = 0x31;
static const u8 KEY_2 = 0x32;
static const u8 KEY_3 = 0x33;
static const u8 KEY_4 = 0x34;
static const u8 KEY_5 = 0x35;
static const u8 KEY_6 = 0x36;
static const u8 KEY_7 = 0x37;
static const u8 KEY_8 = 0x38;
static const u8 KEY_9 = 0x39;
static const u8 KEY_A = 0x41;
static const u8 KEY_B = 0x42;
static const u8 KEY_C = 0x43;
static const u8 KEY_D = 0x44;
static const u8 KEY_E = 0x45;
static const u8 KEY_F = 0x46;
static const u8 KEY_G = 0x47;
static const u8 KEY_H = 0x48;
static const u8 KEY_I = 0x49;
static const u8 KEY_J = 0x4A;
static const u8 KEY_K = 0x4B;
static const u8 KEY_L = 0x4C;
static const u8 KEY_M = 0x4D;
static const u8 KEY_N = 0x4E;
static const u8 KEY_O = 0x4F;
static const u8 KEY_P = 0x50;
static const u8 KEY_Q = 0x51;
static const u8 KEY_R = 0x52;
static const u8 KEY_S = 0x53;
static const u8 KEY_T = 0x54;
static const u8 KEY_U = 0x55;
static const u8 KEY_V = 0x56;
static const u8 KEY_W = 0x57;
static const u8 KEY_X = 0x58;
static const u8 KEY_Y = 0x59;
static const u8 KEY_Z = 0x5A;
static const u8 KEY_UP = VK_UP;
static const u8 KEY_DOWN = VK_DOWN;
static const u8 KEY_LEFT = VK_LEFT;
static const u8 KEY_RIGHT = VK_RIGHT;
static const u8 KEY_ESCAPE = VK_ESCAPE;
static const u8 KEY_ENTER = VK_RETURN;

u64 getSystemTimeInMilliseconds(){
    return GetTickCount64();
}

void getMonitorResolution(u32* x, u32* y){
    *x = GetSystemMetrics(SM_CXSCREEN);
    *y = GetSystemMetrics(SM_CYSCREEN);
}

void displayMessageBox(const char* message){
    MessageBox(0, message, "Message", 0);
}

s8* readFileAsCharacterString(const s8* filename){
    HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    LARGE_INTEGER fileSz;
    GetFileSizeEx(file, &fileSz);
    u64 fileSize = fileSz.QuadPart;
    s8* fileData = new s8[fileSize + 1];
    ReadFile(file, (void*)fileData, fileSize, 0, 0);
    CloseHandle(file);
    fileData[fileSize] = '\0';
    return fileData;
}

u8* readFileAsByteArray(const s8* filename, u64* fileLength){
    HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    LARGE_INTEGER fileSz;
    GetFileSizeEx(file, &fileSz);
    u64 fileSize = fileSz.QuadPart;
    u8* fileData = new u8[fileSize];
    ReadFile(file, (void*)fileData, fileSize, 0, 0);
    CloseHandle(file);
    return fileData;
}

void freeFileData(s8** fileData){
    if(*fileData){
        delete[] *fileData;
        *fileData = 0;
    }
}

void freeFileData(u8** fileData){
    if(*fileData){
        delete[] *fileData;
        *fileData = 0;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)  {  
    if(message == WM_DESTROY || message == WM_CLOSE){
        exit(0);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);  
}

Window createWindow(const s8* title, u32 width, u32 height){
    Window win;
    win.closeRequested = false;

    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.lpszClassName = "BatteryBarrageClass";
    wc.hInstance = GetModuleHandle(0);

    if(!RegisterClass(&wc)){
        MessageBox(0, "Window Registration Failed!", "Error!", 0);
        exit(1);
    }

    u32 mw, mh;
    getMonitorResolution(&mw, &mh);

    u32 wx = (mw - width) / 2;
    u32 wy = (mh - height) / 2;

    HWND window = CreateWindow(wc.lpszClassName, "Battery Barrage", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                               wx, wy, width, height, 
                               0, 0, 0, 0);
    if(window == 0){
        MessageBox(0, "Window Creation Failed!", "Error!", 0);
        exit(1);
    }
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window); 
    SetFocus(window);

    win.window = window;

    return win;
}

void updateWindowEvents(Window* window){
    MSG msg;
    while (PeekMessage(&msg, (HWND)window->window,  0, 0, PM_REMOVE)) { 
        switch(msg.message){
            case WM_KEYDOWN:{
                Key_Inputs[msg.wParam] = true;
                break;
            }
            case WM_KEYUP:{
                Key_Inputs[msg.wParam] = false;
                break;    
            }default:{
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }
        }
    }
}

#endif
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

#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

static const u8 KEY_0 = kVK_ANSI_0;
static const u8 KEY_1 = kVK_ANSI_1;
static const u8 KEY_2 = kVK_ANSI_2;
static const u8 KEY_3 = kVK_ANSI_3;
static const u8 KEY_4 = kVK_ANSI_4;
static const u8 KEY_5 = kVK_ANSI_5;
static const u8 KEY_6 = kVK_ANSI_6;
static const u8 KEY_7 = kVK_ANSI_7;
static const u8 KEY_8 = kVK_ANSI_8;
static const u8 KEY_9 = kVK_ANSI_9;
static const u8 KEY_A = kVK_ANSI_A;
static const u8 KEY_B = kVK_ANSI_B;
static const u8 KEY_C = kVK_ANSI_C;
static const u8 KEY_D = kVK_ANSI_D;
static const u8 KEY_E = kVK_ANSI_E;
static const u8 KEY_F = kVK_ANSI_F;
static const u8 KEY_G = kVK_ANSI_G;
static const u8 KEY_H = kVK_ANSI_H;
static const u8 KEY_I = kVK_ANSI_I;
static const u8 KEY_J = kVK_ANSI_J;
static const u8 KEY_K = kVK_ANSI_K;
static const u8 KEY_L = kVK_ANSI_L;
static const u8 KEY_M = kVK_ANSI_M;
static const u8 KEY_N = kVK_ANSI_N;
static const u8 KEY_O = kVK_ANSI_O;
static const u8 KEY_P = kVK_ANSI_P;
static const u8 KEY_Q = kVK_ANSI_Q;
static const u8 KEY_R = kVK_ANSI_R;
static const u8 KEY_S = kVK_ANSI_S;
static const u8 KEY_T = kVK_ANSI_T;
static const u8 KEY_U = kVK_ANSI_U;
static const u8 KEY_V = kVK_ANSI_V;
static const u8 KEY_W = kVK_ANSI_W;
static const u8 KEY_X = kVK_ANSI_X;
static const u8 KEY_Y = kVK_ANSI_Y;
static const u8 KEY_Z = kVK_ANSI_Z;
static const u8 KEY_UP = kVK_UpArrow;
static const u8 KEY_DOWN = kVK_DownArrow;
static const u8 KEY_LEFT = kVK_LeftArrow;
static const u8 KEY_RIGHT = kVK_RightArrow;
static const u8 KEY_ESCAPE = kVK_Escape;
static const u8 KEY_ENTER = kVK_Return;
static const u8 KEY_SPACE = kVK_Space;

@class WindowDelegate;
@interface WindowDelegate : NSView <NSWindowDelegate> {
}   
@end

@implementation WindowDelegate
-(void)windowWillClose:(NSNotification *)notification {
	[NSApp terminate:self];
}
@end

u64 getSystemTimeInMilliseconds(){
    return [[NSDate date] timeIntervalSince1970]*1000;
}

s8* readFileAsCharacterString(const s8* filename){
    NSError* err  = 0;
    NSString* file = [[NSString alloc] initWithUTF8String: filename];
    NSString* fileContents = [NSString stringWithContentsOfFile: file
                                      encoding: NSUTF8StringEncoding
                                      error: &err];
    if(err){
        [err release];
    }

    u64 len = [fileContents length] + 1;
    s8* str = new s8[len];

    copyMemory((void*)[fileContents UTF8String], (void*)str, len);

    [file release];
    [fileContents release];
    return str;
}

u8* readFileAsByteArray(const s8* filename, u64* fileLength){
    NSString* file = [[NSString alloc] initWithUTF8String: filename];
    NSData* fileData = [NSData dataWithContentsOfFile: file];
    *fileLength = [fileData length];
    u8* data = new u8[*fileLength];
    copyMemory((void*)[fileData bytes], (void*)data, *fileLength);
    
    [file release];
    [fileData release];
    return data;
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

void displayMessageBox(const char* message){
    NSString* string = [[NSString alloc] initWithCString: message
                                        encoding: NSUTF8StringEncoding];
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert setMessageText:string];
    [alert runModal];
    [string release];
    [alert release];
}

Window createWindow(const s8* title, u32 width, u32 height){
    Window win;
    [NSApp sharedApplication]; 
    NSUInteger windowStyle = NSWindowStyleMaskTitled        | 
                             NSWindowStyleMaskClosable      | 
                             NSWindowStyleMaskResizable     | 
                             NSWindowStyleMaskMiniaturizable;

	NSRect screenRect = [[NSScreen mainScreen] frame];
	NSRect viewRect = NSMakeRect(0, 0, width, height); 
	NSRect windowRect = NSMakeRect(NSMidX(screenRect) - NSMidX(viewRect),
								 NSMidY(screenRect) - NSMidY(viewRect),
								 viewRect.size.width, 
								 viewRect.size.height);

	NSWindow * window = [[NSWindow alloc] initWithContentRect:windowRect 
						styleMask:windowStyle
						backing:NSBackingStoreBuffered 
						defer:NO];
    NSWindowController * windowController = [[NSWindowController alloc] initWithWindow:window];

    WindowDelegate* winDelegate = [WindowDelegate alloc];
    [window setDelegate: winDelegate];
    [winDelegate release];
    NSString* str = [[NSString alloc] initWithCString:title encoding: NSASCIIStringEncoding];
    [window setTitle:str];
    [str release];
    [window setAcceptsMouseMovedEvents:YES];
    [window setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];
	[window orderFrontRegardless];  

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    win.window = window;
    win.closeRequested = false;

    [winDelegate release];
    [windowController release];
    return win;
}

void updateWindowEvents(Window* win){
    NSEvent* ev;
    do {
        ev = [NSApp nextEventMatchingMask: NSEventMaskAny
                                untilDate: nil
                                    inMode: NSDefaultRunLoopMode
                                    dequeue: YES];
        if (ev) {
            if([ev type] == NSEventTypeKeyDown){
                Key_Inputs[[ev keyCode]] = true;
            }else if([ev type] == NSEventTypeKeyUp){
                Key_Inputs[[ev keyCode]] = false;
            }else{
                [NSApp sendEvent: ev];
            }
        }
    }while (ev);
}
#endif


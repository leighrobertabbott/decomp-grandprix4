#include <gp4/gp4.h>

#pragma pack(push, 1)
struct CreateMainWindow_Rect {
    int32_t Left;
    int32_t Top;
    int32_t Right;
    int32_t Bottom;
};
struct CreateMainWindow_WndClassEx {
    uint32_t Size;
    uint32_t Style;
    uint32_t WndProc;
    int32_t ClassExtra;
    int32_t WindowExtra;
    uint32_t Instance;
    void* Icon;
    void* Cursor;
    void* Background;
    const char* MenuName;
    const char* ClassName;
    void* SmallIcon;
};
#pragma pack(pop)

typedef void* (__stdcall* LoadIconA_t)(uint32_t instance, uint32_t name);
typedef void* (__stdcall* GetStockObject_t)(int object);
typedef uint16_t (__stdcall* RegisterClassExA_t)(const CreateMainWindow_WndClassEx* windowClass);
typedef int (__stdcall* AdjustWindowRectEx_t)(CreateMainWindow_Rect* rect, uint32_t style, int menu, uint32_t exStyle);
typedef void* (__stdcall* CreateWindowExA_t)(uint32_t exStyle, const char* className, const char* windowName,
                                             uint32_t style, int x, int y, int width, int height, void* parent,
                                             void* menu, uint32_t instance, void* param);
typedef int (__stdcall* ShowWindow_t)(void* window, int command);
typedef int (__stdcall* UpdateWindow_t)(void* window);
typedef void* (__stdcall* SetFocus_t)(void* window);
typedef void* (__stdcall* LoadCursorA_t)(uint32_t instance, uint32_t name);
typedef void* (__stdcall* SetCursor_t)(void* cursor);

// Registers the window class (WNDCLASSEX at 0xb1c814: CS_BYTEALIGNCLIENT, window procedure 0x509250,
// icon resource 0x84, black stock brush) and creates the main window: with the windowed flag
// 0xa6fe68 the client rectangle 0xa6c548 starts at (0x40, 0x40) and is grown by AdjustWindowRectEx
// for WS_POPUP|WS_CAPTION, otherwise it starts at (0, 0) as a WS_POPUP with WS_EX_TOPMOST. The
// rectangle is copied to 0xae669c, the window handle stored in 0xb1c7f0 and the SetFocus result in
// 0xb1c7d8; the arrow cursor is set and 0x50a230 runs. Returns 1.
// FUNCTION: GP4 0x005090a0
int __cdecl CreateMainWindow(uint32_t instance, int unused1, int unused2, const char* className,
                             const char* windowName, int width, int height) {
    CreateMainWindow_Rect& rect = GP4_GLOBAL(CreateMainWindow_Rect, 0x00a6c548);
    CreateMainWindow_WndClassEx& windowClass = GP4_GLOBAL(CreateMainWindow_WndClassEx, 0x00b1c814);
    int left;
    int top;
    if (GP4_GLOBAL(int32_t, 0x00a6fe68) != 0) {
        left = 0x40;
        top = 0x40;
    } else {
        left = 0;
        top = 0;
    }
    rect.Left = left;
    rect.Top = top;
    rect.Right = left + width;
    rect.Bottom = top + height;
    GP4_GLOBAL(uint32_t, 0x00b1c888) = instance;
    windowClass.Size = 0x30;
    windowClass.Style = 0x1000;
    windowClass.WndProc = 0x00509250u;
    windowClass.ClassExtra = 0;
    windowClass.WindowExtra = 0;
    windowClass.Instance = instance;
    windowClass.Icon = GP4_IAT(LoadIconA_t, 0x005f0338)(instance, 0x84);
    windowClass.Cursor = nullptr;
    windowClass.Background = GP4_IAT(GetStockObject_t, 0x005f0034)(4);
    windowClass.MenuName = nullptr;
    windowClass.ClassName = className;
    windowClass.SmallIcon = GP4_IAT(LoadIconA_t, 0x005f0338)(instance, 0x84);
    GP4_IAT(RegisterClassExA_t, 0x005f033c)(&windowClass);

    GP4_GLOBAL(CreateMainWindow_Rect, 0x00ae669c) = rect;
    uint32_t style;
    uint32_t exStyle;
    if (GP4_GLOBAL(int32_t, 0x00a6fe68) != 0) {
        style = 0x80c00000u;
        exStyle = 0;
        GP4_IAT(AdjustWindowRectEx_t, 0x005f0340)(&rect, style, 0, 0);
    } else {
        exStyle = 8;
        style = 0x80000000u;
    }
    void* window = GP4_IAT(CreateWindowExA_t, 0x005f0344)(exStyle, className, windowName, style, rect.Left, rect.Top,
                                                          rect.Right - rect.Left, rect.Bottom - rect.Top, nullptr,
                                                          nullptr, instance, nullptr);
    GP4_GLOBAL(void*, 0x00b1c7f0) = window;
    GP4_IAT(ShowWindow_t, 0x005f0304)(window, 0);
    GP4_IAT(UpdateWindow_t, 0x005f0348)(GP4_GLOBAL(void*, 0x00b1c7f0));
    GP4_GLOBAL(void*, 0x00b1c7d8) = GP4_IAT(SetFocus_t, 0x005f034c)(GP4_GLOBAL(void*, 0x00b1c7f0));
    GP4_IAT(SetCursor_t, 0x005f0354)(GP4_IAT(LoadCursorA_t, 0x005f0350)(0, 0x7f00));
    gp4::cdecl_<0x0050a230, uint32_t>();
    return 1;
}
GP4_IMPL(0x005090a0, CreateMainWindow, "cdecl(u32, i32, i32, ptr[16]:bytes, ptr[16]:bytes, i32[0..2048], i32[0..2048]) -> i32; gfields={0x00a6fe68:u32{=0,1}}; emu=skip:win32 LoadIconA")

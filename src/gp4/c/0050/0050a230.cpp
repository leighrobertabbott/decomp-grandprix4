#include <gp4/gp4.h>
#include <gp4/d3d8.h>

typedef int(__stdcall* ShowCursor_t)(int show);
typedef void*(__stdcall* SetCursor_t)(void* cursor);

static __declspec(noinline) int HideCursorStep() {
    return GP4_IAT(ShowCursor_t, 0x005f0320)(0);
}

// Hides the mouse cursor: calls IDirect3DDevice8::ShowCursor(FALSE) on the device at 0x00a6c0fc (when
// set), calls user32 ShowCursor(FALSE) until the display counter drops below zero, stores the previous
// cursor returned by SetCursor(NULL) at 0x00b2cf04 and clears the "cursor shown" flag 0x00b2cefc.
// Returns whether that flag was set in al (the upper bits of eax keep the SetCursor result).
// FUNCTION: GP4 0x0050a230
uint32_t __cdecl FUN_0050a230() {
    uint32_t wasShown = GP4_GLOBAL(int32_t, 0x00b2cefc) != 0 ? 1u : 0u;
    IDirect3DDevice8* device = GP4_GLOBAL(IDirect3DDevice8*, 0x00a6c0fc);
    if (device != 0) {
        device->ShowCursor(0);
    }
    int count;
    do {
        count = HideCursorStep();
    } while (count >= 0);
    uint32_t previous = (uint32_t)GP4_IAT(SetCursor_t, 0x005f0354)(0);
    GP4_GLOBAL(uint32_t, 0x00b2cf04) = previous;
    GP4_GLOBAL(int32_t, 0x00b2cefc) = 0;
    return (previous & 0xffffff00u) | wasShown;
}
GP4_IMPL(0x0050a230, FUN_0050a230, "cdecl() -> u32; globals=fuzz:i32+layout=d3ddev_00a6c0fc")

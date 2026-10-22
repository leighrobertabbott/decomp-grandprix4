#include <gp4/gp4.h>
#include <gp4/d3d8.h>

// When the flag at 0x00b1c8c8 is set, calls UnlockRect() on the back-buffer surface held in 0x00a6e204.
// Always returns 0.
// FUNCTION: GP4 0x00505da0
int __cdecl FUN_00505da0() {
    if (GP4_GLOBAL(int, 0x00b1c8c8) != 0) {
        GP4_GLOBAL(IDirect3DSurface8*, 0x00a6e204)->UnlockRect();
    }
    return 0;
}
GP4_IMPL(0x00505da0, FUN_00505da0, "cdecl() -> i32; gfields={0x00a6e204:ptr[16]:com=IDirect3DSurface8, 0x00b1c8c8:u32{=0,1}}")

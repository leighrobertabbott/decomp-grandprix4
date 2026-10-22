#include <gp4/gp4.h>
#include <gp4/d3d8.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

// Locks the back-buffer surface (0x00a6e204) when the flag at 0x00b1c8c8 is set. On success the
// locked pixel pointer is stored at 0x00ae6680 and the pitch at 0x00ae670c, and the pointer is
// returned; on failure it writes the "ddraw_lock_from_main_thread() - Failed to lock back buffer"
// message to the debugger. Returns null when the flag is clear or the lock fails.
// FUNCTION: GP4 0x00505d50
void* __cdecl FUN_00505d50() {
    if (GP4_GLOBAL(int, 0x00b1c8c8) != 0) {
        IDirect3DSurface8* surface = GP4_GLOBAL(IDirect3DSurface8*, 0x00a6e204);
        D3DLOCKED_RECT locked;
        if (surface->LockRect(&locked, nullptr, 0) >= 0) {
            GP4_GLOBAL(void*, 0x00ae6680) = locked.pBits;
            GP4_GLOBAL(int32_t, 0x00ae670c) = locked.Pitch;
            return locked.pBits;
        }
        GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0066c96c));
    }
    return nullptr;
}
GP4_IMPL(0x00505d50, FUN_00505d50, "cdecl() -> i32; gfields={0x00a6e204:ptr[16]:com=IDirect3DSurface8, 0x00b1c8c8:u32{=0,1}}")

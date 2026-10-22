#include <gp4/gp4.h>
#include <gp4/d3d8.h>

// Releases the loading-bar texture held in the pointer global 0x00a64c5c (when set) and clears
// the pointer, then clears the progress counter at 0x00a60718.
// FUNCTION: GP4 0x004aa250
void __cdecl FUN_004aa250() {
    IDirect3DTexture8* texture = GP4_GLOBAL(IDirect3DTexture8*, 0x00a64c5c);
    if (texture != nullptr) {
        texture->Release();
        GP4_GLOBAL(IDirect3DTexture8*, 0x00a64c5c) = nullptr;
    }
    GP4_GLOBAL(int32_t, 0x00a60718) = 0;
}
GP4_IMPL(0x004aa250, FUN_004aa250, "cdecl() -> void; gfields={0x00a64c5c:nptr[16]:com=IDirect3DTexture8, 0x00a60718:i32}")

#include <gp4/gp4.h>
#include <gp4/d3d8.h>

// Draws one 7x8 character cell from the 8-byte glyph bitmap [0xa69880] (bit 7 = left column) into the
// back-buffer surface [0xa6e204] (-1 = none) at row [0xa69878], column [0xa6987c] (+1 pixel). With
// the erase byte [0xa69884] clear, set bits become the text colour and clear bits 0; with it set, set
// bits are cleared. Surface format [0xa6c528] 0x17/0x18 (R5G6B5 / X1R5G5B5) takes the 16-bit path
// (colour by the byte 0x65f065), anything else 32-bit pixels 0xffffcf. The lock result is not checked.
// FUNCTION: GP4 0x004a97c0
int __cdecl FUN_004a97c0() {
    IDirect3DSurface8* surface = GP4_GLOBAL(IDirect3DSurface8*, 0x00a6e204);
    const int32_t pitchRow = GP4_GLOBAL(int32_t, 0x00a69878);
    const int32_t column = GP4_GLOBAL(int32_t, 0x00a6987c);
    const uint8_t* glyph = GP4_GLOBAL(const uint8_t*, 0x00a69880);
    const uint8_t erase = GP4_GLOBAL(uint8_t, 0x00a69884);
    if (surface == reinterpret_cast<IDirect3DSurface8*>(-1)) {
        return 0;
    }
    D3DLOCKED_RECT locked;
    surface->LockRect(&locked, nullptr, 0);
    const int32_t format = GP4_GLOBAL(int32_t, 0x00a6c528);
    if (format != 0x17 && format != 0x18) {
        uint32_t* line = static_cast<uint32_t*>(locked.pBits) + (locked.Pitch / 4) * pitchRow + column + 1;
        for (int32_t rows = 8; rows != 0; rows--) {
            uint8_t mask = 0x80;
            const uint8_t bits = *glyph;
            uint32_t* pixel = line;
            for (int32_t columns = 7; columns != 0; columns--) {
                if (erase == 0) {
                    *pixel = (mask & bits) ? 0xffffcfu : 0u;
                } else if ((bits & mask) != 0) {
                    *pixel = 0;
                }
                mask >>= 1;
                pixel++;
            }
            glyph++;
            line += locked.Pitch / 4;
        }
        surface->UnlockRect();
        return 0;
    }
    uint32_t red;
    uint32_t green;
    if (GP4_GLOBAL(uint8_t, 0x0065f065) != 0) {
        red = 0xf800;
        green = 0x7e0;
    } else {
        red = 0x7be8;
        green = 0x3e0;
    }
    uint16_t* line = static_cast<uint16_t*>(locked.pBits) + (static_cast<int16_t>(locked.Pitch) / 2) * pitchRow + column + 1;
    for (int32_t rows = 8; rows != 0; rows--) {
        uint8_t mask = 0x80;
        const uint8_t bits = *glyph;
        uint16_t* pixel = line;
        for (int32_t columns = 7; columns != 0; columns--) {
            if (erase == 0) {
                if ((bits & mask) != 0) {
                    *pixel = static_cast<uint16_t>(green | red);
                } else {
                    *pixel = 0;
                }
            } else if ((bits & mask) != 0) {
                *pixel = 0;
            }
            mask >>= 1;
            pixel++;
        }
        glyph++;
        line += locked.Pitch / 2;
    }
    surface->UnlockRect();
    return 0;
}
GP4_IMPL(0x004a97c0, FUN_004a97c0, "cdecl() -> i32; gfields={0x00a6e204:ptr[16]:com=IDirect3DSurface8, 0x00a6c528:u32{=0x16,0x17,0x18}, 0x00a69878:i32[0..8], 0x00a6987c:i32[0..8], 0x00a69880:ptr[0x10]:bytes, 0x00a69884:u8{=0,1}, 0x0065f065:u8{=0,1}}; comfail=off")

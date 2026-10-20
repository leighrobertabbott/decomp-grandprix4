#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws a textured rectangle into the 8-bit surface [0x00874ee4] (row pitch [0x008751c4]): ecx =
// (x centre << 16) | width, ebp = (y centre << 16) | height, eax = slot index of the word table
// 0x009e1b20 (0xffff = no texture, else the byte offset of a 0x20-byte entry of the texture table
// 0x006975ec: +0 texel pointer, words +4/+6 = width/height). esi/edi are the 8.24 texture u/v at the
// centre and ebx/edx their steps per pixel/row; the start is moved back by half the size. Each pixel
// takes texel [((v >> 24) << 8 | (u >> 24)) & mask], mask = ((width-1) & 0xff) | ((height-1) & 0xff) << 8
// (kept at 0x009e7da0). u/v and the steps live in 0x009658f0/f4 and 0x009e7d8c/90 (u is reset per row).
// pushal/popal: registers are restored; the flags are those of the last flag writer.

// FUNCTION: GP4 0x0051f555
void __cdecl FUN_0051f555(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x009658f0) = r->esi;
    GP4_GLOBAL(uint32_t, 0x009658f4) = r->edi;
    GP4_GLOBAL(uint32_t, 0x009e7d8c) = r->ebx;
    GP4_GLOBAL(uint32_t, 0x009e7d90) = r->edx;

    const uint32_t y_offset = 0u - (uint32_t)((int32_t)(r->ebp & 0xffffu) >> 1);
    const uint32_t x_offset = 0u - (uint32_t)((int32_t)(r->ecx & 0xffffu) >> 1);
    uint32_t dest = ((r->ebp >> 16) + y_offset) * GP4_GLOBAL(uint32_t, 0x008751c4);
    dest += r->ecx >> 16;
    dest += x_offset;
    dest += GP4_GLOBAL(uint32_t, 0x00874ee4);
    GP4_GLOBAL(uint32_t, 0x009658f0) += x_offset * GP4_GLOBAL(uint32_t, 0x009e7d8c);
    GP4_GLOBAL(uint32_t, 0x009658f4) += y_offset * GP4_GLOBAL(uint32_t, 0x009e7d90);

    const uint32_t width = r->ecx & 0xffffu;
    const uint32_t height = r->ebp & 0xffffu;
    const uint16_t slot = GP4_ARRAY(uint16_t, 0x009e1b20)[r->eax];
    uint32_t flags;
    if (slot == 0xffffu) {
        flags = gp4::flags::sub16(slot, 0xffffu);
    } else {
        const uint8_t* entry = GP4_ARRAY(uint8_t, 0x006975ec) + slot;
        const uint32_t mask = (((uint32_t)*(const uint16_t*)(entry + 4) - 1u) & 0xffu)
                            | ((((uint32_t)*(const uint16_t*)(entry + 6) - 1u) & 0xffu) << 8);
        GP4_GLOBAL(uint32_t, 0x009e7da0) = mask;
        const uint8_t* texels = *(const uint8_t* const*)entry;
        if (height == 0u) {
            flags = gp4::flags::logic32(height);
        } else if (width == 0u) {
            flags = gp4::flags::logic32(width);
        } else {
            const uint32_t u_start = GP4_GLOBAL(uint32_t, 0x009658f0);
            const uint32_t du = GP4_GLOBAL(uint32_t, 0x009e7d8c);
            const uint32_t dv = GP4_GLOBAL(uint32_t, 0x009e7d90);
            const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x008751c4);
            uint32_t v = GP4_GLOBAL(uint32_t, 0x009658f4);
            uint32_t rows = height;
            for (;;) {
                uint8_t* out = (uint8_t*)(uintptr_t)dest;
                uint32_t u = u_start;
                for (uint32_t n = width; n != 0u; --n) {
                    const uint32_t index = (((v >> 24) << 8) | (u >> 24)) & mask;
                    *out++ = texels[index];
                    u += du;
                }
                dest += pitch;
                flags = gp4::flags::add32(v, dv);
                v += dv;
                flags = gp4::flags::dec32(rows, flags);
                if (--rows == 0u) {
                    break;
                }
            }
            GP4_GLOBAL(uint32_t, 0x009658f4) = v;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051f555, FUN_0051f555, "regs(eax:u32[0..63], ebx:u32, ecx:u32{=0x00200000,0x00200001,0x00200040,0x00300011,0x00400020,0x00280007,0x00340003}, edx:u32, esi:u32, edi:u32, ebp:u32{=0x00080000,0x00080001,0x00080010,0x000c0007,0x00100010,0x000a0003,0x000b0002}) -> (cf, pf, af, zf, sf, of); globals=layout=surf_0051f4bd+slots_0051cc50+tex_0051c6c3+tex_0051c690")

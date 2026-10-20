#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inverse of 0x005324a4: the five bytes at 0x827f69 each hold the top two bits of four samples. For every
// byte b (lodsb) the four samples at 0x7c2a48.. get their top two bits replaced (b rotated left by 2 per
// sample: first sample takes bits 7..6). Leaves esi = 0x827f6e, edi = 0x7c2a5c, ecx = 0, al = the last
// source byte, ah = its two low bits moved to the top. The last flag writer is the final `rol al, 2`
// (CF = new bit 0, OF = msb ^ CF); SF/ZF/AF/PF stay those of the preceding `inc edi` (0x7c2a5b -> 0x7c2a5c).
// FUNCTION: GP4 0x005324cc
void __cdecl FUN_005324cc(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x00827f69);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c2a48);

    uint32_t last = 0;
    uint32_t top = 0;
    for (int group = 0; group < 5; ++group) {
        last = *src++;
        uint32_t bits = last;
        for (int i = 0; i < 4; ++i) {
            top = bits & 0xc0u;
            *dst = static_cast<uint8_t>((*dst & 0x3fu) | top);
            ++dst;
            bits = ((bits << 2) | (bits >> 6)) & 0xffu;
        }
    }

    r->eax = (r->eax & 0xffff0000u) | (top << 8) | last;
    r->ecx = 0u;
    r->esi = reinterpret_cast<uint32_t>(src);
    r->edi = reinterpret_cast<uint32_t>(dst);
    const uint32_t carry = last & 1u;
    const uint32_t overflow = ((last >> 7) ^ last) & 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::PF | carry | (overflow ? gp4::flags::OF : 0u));
}
GP4_IMPL(0x005324cc, FUN_005324cc, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv22_0052d402+pack_00827f69")

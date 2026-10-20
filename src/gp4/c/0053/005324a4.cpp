#include <gp4/gp4.h>

// FUNCTION: GP4 0x005324a4
// Packs 20 six-bit samples into 5 bytes of high bits: for each group of four source bytes at
// 0x7c2a48 the top two bits of every byte are collected (rol ax,2 per byte, first byte ends up
// highest) and stored as one byte at 0x827f69+group; the source bytes are masked to 6 bits in place.
// Leaves esi = 0x7c2a5c, edi = 0x827f6e, ecx = 0, ax = last packed byte in both halves; the flags
// are those of the last `inc esi` (CF still clear from the preceding `and`).
void __cdecl FUN_005324a4(gp4::Regs* r) {
    uint8_t* src = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x00827f69);
    uint32_t ax = r->eax & 0xffffu;

    for (int group = 0; group < 5; ++group) {
        for (int i = 0; i < 4; ++i) {
            ax = (ax & 0xff00u) | *src;                      // mov al, [esi]
            ax = ((ax << 2) | (ax >> 14)) & 0xffffu;         // rol ax, 2
            *src = (uint8_t)(*src & 0x3fu);                  // and byte [esi], 0x3f
            ++src;                                           // inc esi
        }
        ax = (ax & 0xff00u) | (ax >> 8);                     // mov al, ah
        *dst++ = (uint8_t)ax;                                // stosb
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->ecx = 0u;
    r->esi = (uint32_t)src;
    r->edi = (uint32_t)dst;
    // Last flag writer: inc esi (0x7c2a5b -> 0x7c2a5c): ZF = SF = OF = AF = 0, PF = 1 (0x5c has
    // four set bits); CF stays clear from the `and` before it.
    r->eflags = (r->eflags & ~0x8d5u) | 0x04u;
}
GP4_IMPL(0x005324a4, FUN_005324a4, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

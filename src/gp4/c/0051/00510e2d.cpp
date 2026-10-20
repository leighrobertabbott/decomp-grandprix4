#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bit-set synchroniser: cl is a bit number (0..255 -> byte cl >> 3, mask 1 << (cl & 7)) into two
// 32-byte bitmaps, 0x008245a0 (wanted) and 0x008910d5 (state).
//   wanted bit clear: the state bit is cleared; cl = ~mask | 0x80
//   wanted set, state already set: nothing changes; cl = mask | 0x80
//   wanted set, state clear: the state bit is set; cl = 0
// Only cl changes; the flags are those of the final `and cl,0` / `or cl,0x80`.
// FUNCTION: GP4 0x00510e2d
void __cdecl FUN_00510e2d(gp4::Regs* r) {
    const uint32_t bit = r->ecx & 0xffu;
    const uint32_t index = bit >> 3;
    const uint32_t mask = 1u << (bit & 7u);
    const uint8_t* wanted = GP4_ARRAY(uint8_t, 0x008245a0);
    uint8_t* state = GP4_ARRAY(uint8_t, 0x008910d5);

    uint32_t cl;
    if ((wanted[index] & mask) == 0u) {
        state[index] = (uint8_t)(state[index] & ~mask);
        cl = (~mask & 0xffu) | 0x80u;
    } else if ((state[index] & mask) == 0u) {
        state[index] = (uint8_t)(state[index] | mask);
        cl = 0u;
    } else {
        cl = mask | 0x80u;
    }
    r->ecx = (r->ecx & 0xffffff00u) | cl;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cl));
}
GP4_IMPL(0x00510e2d, FUN_00510e2d, "regs(ecx:u32) -> (ecx, cf, pf, af, zf, sf, of); globals=layout=bitmap_00510e2d")

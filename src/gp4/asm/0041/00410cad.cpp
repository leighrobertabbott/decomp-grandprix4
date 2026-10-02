#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410cad
// Writes 6 bytes at edx (plus a zero terminator at +6): a linear ramp from byte [eax+0]
// to byte [eax+6] in 16.16 fixed point, rounded, in 5 steps. eax and edx may alias.
void __cdecl FUN_00410cad(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));

    const int32_t end = static_cast<int32_t>(static_cast<uint32_t>(src[6]) << 16);
    const int32_t start = static_cast<int32_t>(static_cast<uint32_t>(src[0]) << 16);
    const int32_t step = (end - start) / 5;

    int32_t value = start;
    for (int i = 0; i < 6; ++i) {
        dst[i] = static_cast<uint8_t>((static_cast<uint32_t>(value) + 0x8000u) >> 16);
        value += step;
    }
    dst[6] = 0;

    // the loop ends on cmp ecx, 6 with ecx = 6: ZF = PF = 1, the rest clear; popal keeps the flags
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00410cad, FUN_00410cad, "regs(eax:ptr[0x10]:bytes, edx:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of)")

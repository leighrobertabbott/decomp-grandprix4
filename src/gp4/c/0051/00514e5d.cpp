#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: with [0x88fe12] = 0xff it writes the seven characters "PFLT.:." through the
// readout writer 0x0051f3bd, each one with the cursor bytes 0x88fd94..97 zeroed and the dword
// [0x88fe14] set to 6, 7, ..., 12 in turn (the registers the writer leaves are threaded into the
// next call, as in the original). Afterwards [0x88fe14] = -1 and [0x88fe12] = 0. The registers come
// back unchanged (popal); the flags are those the last writer call leaves.
// FUNCTION: GP4 0x00514e5d
void __cdecl FUN_00514e5d(gp4::Regs* r) {
    static const uint8_t kChars[7] = {0x50u, 0x46u, 0x4cu, 0x54u, 0x2eu, 0x3au, 0x2eu};
    gp4::Regs c = *r;
    GP4_GLOBAL(uint8_t, 0x0088fe12) = 0xffu;
    for (uint32_t k = 0; k < 7u; ++k) {
        GP4_GLOBAL(uint8_t, 0x0088fd94) = 0u;
        GP4_GLOBAL(uint8_t, 0x0088fd95) = 0u;
        GP4_GLOBAL(uint8_t, 0x0088fd96) = 0u;
        GP4_GLOBAL(uint8_t, 0x0088fd97) = 0u;
        GP4_GLOBAL(uint32_t, 0x0088fe14) = 6u + k;
        c.eax = (c.eax & 0xffffff00u) | kChars[k];
        gp4::call_regs(0x0051f3bd, c);
    }
    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;
    GP4_GLOBAL(uint8_t, 0x0088fe12) = 0u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00514e5d, FUN_00514e5d, "regs(eax:u32, edi:ptr[0x40]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")

#include <gp4/gp4.h>

// Clears sixteen 0x48-byte records' first dword (the unaligned dwords at 0x008300db + k*0x48),
// then zeroes the dwords at 0x0083055b (= 0x8300db + 16*0x48), 0x0083059f and 0x008305a3.
// Hand-written register code: leaves ebx = 0x0083055b (one past the last record), ecx = 0 and the
// flags of the loop's final `or ecx, ecx` (ZF = PF = 1, CF = OF = SF = AF = 0).
// FUNCTION: GP4 0x0053ee80
void __cdecl FUN_0053ee80(gp4::Regs* r) {
    uint32_t p = 0x008300dbu;
    for (int k = 0; k < 16; ++k) {
        GP4_GLOBAL(uint32_t, p) = 0u;
        p += 0x48u;
    }
    GP4_GLOBAL(uint32_t, 0x0083055b) = 0u;
    GP4_GLOBAL(uint32_t, 0x0083059f) = 0u;
    GP4_GLOBAL(uint32_t, 0x008305a3) = 0u;

    r->ebx = p;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;   // PF | ZF
}
GP4_IMPL(0x0053ee80, FUN_0053ee80, "regs() -> (ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

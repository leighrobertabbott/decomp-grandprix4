#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040860a
void __cdecl FUN_0040860a(gp4::Regs* r) {
    for (int i = 0; i < 22; i++) {
        GP4_ARRAY(uint8_t, 0x0066d90a)[i] = 0;
        GP4_ARRAY(uint32_t, 0x0066fb80)[i] = 0x21980000u;
        GP4_ARRAY(uint32_t, 0x0066fbd8)[i] = 0x21980000u;
        GP4_ARRAY(uint32_t, 0x0066fc30)[i] = 0x21980000u;
        GP4_ARRAY(uint32_t, 0x0066fc88)[i] = 0x21980000u;
        GP4_ARRAY(uint8_t, 0x0066ec4a)[i] = 0;
        GP4_ARRAY(uint8_t, 0x0066d920)[i] &= 0xfe;
    }
    // The original leaves its loop counters behind: esi = 22 (last inc), ecx = 0 (loop).
    // Last flag writer is "inc esi" (21 -> 22) after "and byte, 0xfe" (CF = 0): all six clear.
    r->esi = 22;
    r->ecx = 0;
    r->eflags &= ~0x8d5u;
}
GP4_IMPL(0x0040860a, FUN_0040860a, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of)")

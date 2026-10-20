#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040cd8b
// pushal / popal wrapper (all registers restored; the flags of the last flag writer, the callee
// 0x0040cbf7, remain): clears the flag byte 0x00604430 and the 0x600-byte block at 0x006269b8, takes
// the per-circuit data pointer 0x0062d2e4[[0x007ad894]], lets 0x0040c6d8 and then 0x0040cbf7 parse
// it (each advances esi) and stores the final esi in 0x0062d280.
void __cdecl FUN_0040cd8b(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00604430) = 0;
    uint8_t* const block = GP4_ARRAY(uint8_t, 0x006269b8);
    for (uint32_t i = 0; i < 0x600u; ++i) {
        block[i] = 0;
    }
    const uint32_t source = GP4_ARRAY(const uint32_t, 0x0062d2e4)[GP4_GLOBAL(uint32_t, 0x007ad894)];

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = 0;
    call.ecx = 0;
    call.edi = 0x006269b8u + 0x600u;
    call.esi = source;
    gp4::call_regs(0x0040c6d8, call);
    gp4::call_regs(0x0040cbf7, call);
    GP4_GLOBAL(uint32_t, 0x0062d280) = call.esi;

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040cd8b, FUN_0040cd8b, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=trk_007ad894")

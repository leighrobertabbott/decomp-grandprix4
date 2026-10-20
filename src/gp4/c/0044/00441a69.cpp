#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// When the sign bit of the flag byte at 0x0066d87f is set, runs the control-block / segment-record setup
// 0x00441a78; otherwise only the flags of `test byte [0x66d87f], 0xff` remain.
// FUNCTION: GP4 0x00441a69
void __cdecl FUN_00441a69(gp4::Regs* r) {
    const uint8_t value = GP4_GLOBAL(uint8_t, 0x0066d87f);
    if ((value & 0x80u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(value);
        return;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00441a78, call);

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00441a69, FUN_00441a69, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+ptr_0086850c")

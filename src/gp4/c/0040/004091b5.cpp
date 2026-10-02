#include <gp4/gp4.h>

// FUNCTION: GP4 0x004091b5
void __cdecl FUN_004091b5(gp4::Regs* r) {
    uint16_t* table = GP4_ARRAY(uint16_t, 0x0066fe3c);
    uint32_t value = 0;
    for (int i = 0; i < 22; i++) {
        table[i] = static_cast<uint16_t>(value);
        value += 0x41c;
    }
    GP4_GLOBAL(uint16_t, 0x00679b00) = 0x2c;
    table[-1] = 0xffff;
    table[22] = 0xffff;

    r->eax = 0x2c;
    r->ecx = 0;
    r->esi = 0x0066fe3c + 0x2c;
    // the final add esi, eax (0x66fe3c + 0x2c) leaves only AF set
    r->eflags = (r->eflags & ~0x8d5u) | 0x10u;
}
GP4_IMPL(0x004091b5, FUN_004091b5, "regs() -> (eax, ecx, esi, cf, pf, af, zf, sf, of)")

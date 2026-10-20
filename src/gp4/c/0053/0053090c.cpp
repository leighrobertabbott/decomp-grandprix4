#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053090c
// Marks [0x7a97a4] + 1 records of 0x3c bytes from 0x00827160: byte +5 = 1 and the dwords +0xc and +0x10
// = -1. Leaves ebx at the first record past the last one and ecx = 0; the flags are those of the final
// `dec ecx` (1 -> 0), carrying the CF of the last `add ebx, 0x3c`.
void __cdecl FUN_0053090c(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a97a4) + 1u;
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x00827160);
    uint32_t ebx = 0x00827160u;
    uint32_t arith;
    do {
        rec[5] = 1u;
        GP4_FIELD(uint32_t, rec, 0xc) = 0xffffffffu;
        GP4_FIELD(uint32_t, rec, 0x10) = 0xffffffffu;
        arith = gp4::flags::add32(ebx, 0x3cu);
        ebx += 0x3cu;
        rec += 0x3c;
        arith = gp4::flags::dec32(count, arith);
        count -= 1u;
    } while (count != 0u);
    r->ebx = ebx;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053090c, FUN_0053090c, "regs() -> (ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts the word at 0x007c2ea0 (minutes) to milliseconds: [0x00679b04] = word * 60000.
// eax holds the product on return; the flags are those of `imul eax, eax, 0xea60`: ZF/SF/PF from the
// 32-bit product, AF = 0, CF = OF = 1 when the signed product overflows (word >= 35792).
// FUNCTION: GP4 0x0051feef
void __cdecl FUN_0051feef(gp4::Regs* r) {
    const uint32_t minutes = GP4_GLOBAL(uint16_t, 0x007c2ea0);
    const uint32_t product = minutes * 0xea60u;
    uint32_t arith = gp4::flags::logic32(product);
    if (product >= 0x80000000u) {
        arith |= gp4::CF | 0x800u;
    }
    GP4_GLOBAL(uint32_t, 0x00679b04) = product;
    r->eax = product;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051feef, FUN_0051feef, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

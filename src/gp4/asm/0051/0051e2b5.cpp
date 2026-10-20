#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reports through CF whether the input selected by bl is active: bl == 2 tests bit 4 of the byte at
// 0x679d1b, bl == 3 tests bit 1 of the byte at 0x679d1c, any other bl asks 0x0051ddda (a bit selected
// through the byte table at 0x6f2d90) and takes it as active when that routine ends with CF or ZF set.
// CF = 1 when active (`stc`), 0 otherwise (`clc`); the other flags are those of the last test/compare.
// FUNCTION: GP4 0x0051e2b5
void __cdecl FUN_0051e2b5(gp4::Regs* r) {
    const uint32_t mode = r->ebx & 0xffu;
    uint32_t arith;
    uint32_t carry;

    if (mode == 2u) {
        const uint32_t bits = GP4_GLOBAL(uint8_t, 0x00679d1b) & 0x10u;
        arith = gp4::flags::logic8(bits);
        carry = (bits != 0u) ? 1u : 0u;
    } else if (mode == 3u) {
        const uint32_t bits = GP4_GLOBAL(uint8_t, 0x00679d1c) & 0x02u;
        arith = gp4::flags::logic8(bits);
        carry = (bits != 0u) ? 1u : 0u;
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 3u));
        gp4::call_regs(0x0051ddda, *r);
        arith = r->eflags;
        carry = ((arith & gp4::flags::CF) != 0u || (arith & gp4::flags::ZF) != 0u) ? 1u : 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~static_cast<uint32_t>(gp4::flags::CF)) | carry);
}
GP4_IMPL(0x0051e2b5, FUN_0051e2b5, "regs(ebx:u32{2,3,2,3,0,1,4,0x41,0xff,0x7f}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_006f2d90")

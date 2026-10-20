#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00555bec
// pushal ... popal wrapper. Fills the 32 dword entries at 0x00874d38 with edi - 0x160 and the 32
// bytes at 0x00874db8 with 0xff, then clears the four bytes 0x00874dcd..0x00874dd0. All
// registers are restored; the flags are those of the last `cmp ecx, 0x20` (ecx = 0x20).
void __cdecl FUN_00555bec(gp4::Regs* r) {
    const uint32_t value = r->edi - 0x160u;
    uint32_t* entries = GP4_ARRAY(uint32_t, 0x00874d38);
    uint8_t* marks = GP4_ARRAY(uint8_t, 0x00874db8);

    for (uint32_t i = 0; i != 0x20u; ++i) {
        entries[i] = value;
        marks[i] = 0xffu;
    }

    GP4_GLOBAL(uint8_t, 0x00874dd0) = 0;
    GP4_GLOBAL(uint8_t, 0x00874dcf) = 0;
    GP4_GLOBAL(uint8_t, 0x00874dce) = 0;
    GP4_GLOBAL(uint8_t, 0x00874dcd) = 0;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x20u, 0x20u));
}
GP4_IMPL(0x00555bec, FUN_00555bec, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

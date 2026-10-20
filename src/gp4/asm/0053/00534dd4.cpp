#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the dword at ebx+4 to the (unaligned) dword 0x00828297 and bumps the (unaligned) counter
// dword 0x00828293. eax keeps the copied dword; the flags are those of the `add dword [..], 1`.
// FUNCTION: GP4 0x00534dd4
void __cdecl FUN_00534dd4(gp4::Regs* r) {
    const uint32_t value = GP4_FIELD(uint32_t, r->ebx, 4);

    GP4_GLOBAL(uint32_t, 0x00828297) = value;
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
    GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;

    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 1u));
}
GP4_IMPL(0x00534dd4, FUN_00534dd4, "regs(ebx:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

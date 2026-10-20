#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests a flag bit selected through a byte table: t = byte [0x006f2d90 + bl]. t == 0xff means "no
// bit": CF = 1, eax = bl, the other flags are those of `cmp t, 0xff` (equal). Otherwise bit (t &
// 0x1f) of the dword [0x007a979c] is tested: set gives eax = 0xff (flags of `or al, 0xff`), clear
// gives eax = 0 (flags of `and al, 0`), CF = 0 both ways.
// FUNCTION: GP4 0x0051ddda
void __cdecl FUN_0051ddda(gp4::Regs* r) {
    const uint32_t index = r->ebx & 0xffu;
    const uint8_t entry = GP4_ARRAY(const uint8_t, 0x006f2d90)[index];
    if (entry == 0xffu) {
        r->eax = index;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(entry, 0xffu) | gp4::flags::CF);
        return;
    }
    const uint32_t bit = (GP4_GLOBAL(uint32_t, 0x007a979c) >> (entry & 0x1fu)) & 1u;
    if (bit != 0u) {
        r->eax = 0xffu;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0xffu));
    } else {
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u));
    }
}
GP4_IMPL(0x0051ddda, FUN_0051ddda, "regs(ebx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

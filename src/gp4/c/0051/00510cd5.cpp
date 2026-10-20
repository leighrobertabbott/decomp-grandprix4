#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the 256-entry colour conversion table at 0x006e2568: entry i is the packed 16-bit colour of
// palette entry i (0x00510c88) in both halves of the dword. pushal/popal restore every GPR but not the
// flags, which are those of the final `cmp ecx, 0x100` (equal: ZF and PF).
// FUNCTION: GP4 0x00510cd5
void __cdecl FUN_00510cd5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    uint32_t* table = GP4_ARRAY(uint32_t, 0x006e2568);
    for (uint32_t index = 0u; index != 0x100u; ++index) {
        c.ecx = index;
        c.eax = (c.eax & 0xffffff00u) | index;
        gp4::call_regs(0x00510c88, c);
        const uint32_t packed = c.eax & 0xffffu;
        c.eax = (packed << 16) | packed;
        table[index] = c.eax;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x100u, 0x100u));
}
GP4_IMPL(0x00510cd5, FUN_00510cd5, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

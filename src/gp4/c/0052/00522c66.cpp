#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the 22 dwords at 0x007c3208 (`loop`; ebx walks to 0x007c3260), runs the 22-byte list copy 0x00522c96
// (pushal/popal, registers unchanged), then clears 0x882 bytes from 0x007c3260. ebx ends as 0x007c3260 and
// ecx as 0x882; the flags are those of the final `cmp ecx, 0x882` (equal).
// FUNCTION: GP4 0x00522c66
void __cdecl FUN_00522c66(gp4::Regs* r) {
    uint32_t* words = GP4_ARRAY(uint32_t, 0x007c3208);
    for (uint32_t i = 0; i != 0x16u; ++i) {
        words[i] = 0u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ebx = 0x007c3260u;
    c.ecx = 0u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x007c3258u, 4u));
    gp4::call_regs(0x00522c96, c);

    uint8_t* bytes = GP4_ARRAY(uint8_t, 0x007c3260);
    for (uint32_t i = 0; i != 0x882u; ++i) {
        bytes[i] = 0u;
    }

    r->ebx = 0x007c3260u;
    r->ecx = 0x882u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x882u, 0x882u));
}
GP4_IMPL(0x00522c66, FUN_00522c66, "regs() -> (ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_0066d8f4")

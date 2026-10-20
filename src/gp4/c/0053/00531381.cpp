#include <gp4/gp4.h>
#include <gp4/flags.h>

// Leaves the callee registers in *r and ends with `stc` (`carry` = 1) or `clc` (`carry` = 0): the
// other arithmetic flags are those of the last flag writer (`arith`).
static void Finish(gp4::Regs* r, const gp4::Regs& c, uint32_t arith, uint32_t carry) {
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~gp4::flags::CF) | carry);
}

// Device-record check: unless the byte flags [0x7aafd3] / [0x7ab020] are set, tests the device bitmap
// bit of the current selector (0x531847, CF = bit; a clear bit exits), looks the selector up in the
// device records (0x5313bd; a miss exits) and checks that the column's bit is clear in byte +6 of the
// found record. CF = 0 (clc) only when every step passes; otherwise CF = 1 (stc) with the other flags
// of the last flag writer. eax, ebx, ecx and edx are those left by the callees (and by the final
// `mov ecx, edx; mov eax, 1; shl eax, cl`).
// FUNCTION: GP4 0x00531381
void __cdecl FUN_00531381(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint8_t first = GP4_GLOBAL(uint8_t, 0x007aafd3);
    if (first != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(first, 0u) | gp4::flags::CF);
        return;
    }
    const uint8_t second = GP4_GLOBAL(uint8_t, 0x007ab020);
    if (second != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(second, 0u) | gp4::flags::CF);
        return;
    }

    gp4::call_regs(0x00531847, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        Finish(r, c, c.eflags, gp4::flags::CF);
        return;
    }
    gp4::call_regs(0x00530774, c);
    gp4::call_regs(0x005313bd, c);
    if ((c.eflags & gp4::flags::CF) != 0u) {
        Finish(r, c, c.eflags, gp4::flags::CF);
        return;
    }

    c.ecx = c.edx;
    c.eax = 1u << (c.ecx & 31u);
    const uint32_t arith = gp4::flags::logic8(GP4_FIELD(uint8_t, c.ebx, 6) & (c.eax & 0xffu));
    Finish(r, c, arith, (GP4_FIELD(uint8_t, c.ebx, 6) & (c.eax & 0xffu)) != 0u ? gp4::flags::CF : 0u);
}
GP4_IMPL(0x00531381, FUN_00531381, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648+bits_007ad844+sel_00531859+flag_007ab020+mask6_00531381")

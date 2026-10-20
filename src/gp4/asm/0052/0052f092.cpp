#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws three entries of the text table (each lookup + draw is one call of 0x0054d1a2, which turns the index in
// esi into the string pointer 0x00835846[esi] and hands it to the text drawer): lodsb takes a byte from [esi]
// (clamped to 0x10, esi advanced by one) and the entries 0xa7c + n, 0xb3 and 0xac0 + n are drawn in that order,
// then 0x0052f1f0 advances the line counter. The advanced esi is restored at the end; every other register and
// the flags are whatever the last callee (0x0052f1f0) leaves. Assumes DF clear for the lodsb.
// FUNCTION: GP4 0x0052f092
void __cdecl FUN_0052f092(gp4::Regs* r) {
    const uint32_t advanced = r->esi + 1u;
    uint32_t count = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(count, 0x10u));
    if (count > 0x10u) {
        count = 0x10u;
    }
    c.eax = (r->eax & 0xffffff00u) | count;

    c.esi = count + 0xa7cu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(count, 0xa7cu));
    gp4::call_regs(0x0054d1a2, c);

    c.esi = 0xb3u;
    gp4::call_regs(0x0054d1a2, c);

    c.esi = count + 0xac0u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(count, 0xac0u));
    gp4::call_regs(0x0054d1a2, c);

    gp4::call_regs(0x0052f1f0, c);

    c.esi = advanced;
    *r = c;
}
GP4_IMPL(0x0052f092, FUN_0052f092, "regs(esi:ptr[0x40]:u8[0..0x30]) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=layout=tab_00835846+lim_0052f1f0")

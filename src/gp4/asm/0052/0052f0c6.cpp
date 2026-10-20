#include <gp4/gp4.h>
#include <gp4/flags.h>

// Text-table dispatcher: lodsb takes a command byte from [esi] (esi advances by one) and stores it at 0x826e3e;
// the byte selects a text-table index in esi (0 -> 0x4b2, 1 -> 0x4b3, 3 -> 0x4b5, 0x44 -> 0x4b6, 2 -> 0x4b7,
// 0x80 -> 0x4b8, anything else -> 0). That entry is drawn through 0x0054d1a2 (esi = index into the table at
// 0x00835846) and 0x0052f1f0 then advances the line counter. The advanced esi is restored at the end; the other
// registers and the flags are whatever the last callee leaves. The flags handed to the first callee are those of
// the last compare executed in the chain. Assumes DF clear for the lodsb.
// FUNCTION: GP4 0x0052f0c6
void __cdecl FUN_0052f0c6(gp4::Regs* r) {
    const uint32_t advanced = r->esi + 1u;
    const uint32_t al = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint8_t, 0x00826e3e) = static_cast<uint8_t>(al);

    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | al;

    uint32_t index;
    uint32_t last;
    if (al == 0x00u) {
        index = 0x4b2u;
        last = 0x00u;
    } else if (al == 0x01u) {
        index = 0x4b3u;
        last = 0x01u;
    } else if (al == 0x03u) {
        index = 0x4b5u;
        last = 0x03u;
    } else if (al == 0x44u) {
        index = 0x4b6u;
        last = 0x44u;
    } else if (al == 0x02u) {
        index = 0x4b7u;
        last = 0x02u;
    } else if (al == 0x80u) {
        index = 0x4b8u;
        last = 0x80u;
    } else {
        index = 0u;
        last = 0x80u;
    }
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(al, last));
    c.esi = index;
    gp4::call_regs(0x0054d1a2, c);
    gp4::call_regs(0x0052f1f0, c);

    c.esi = advanced;
    *r = c;
}
GP4_IMPL(0x0052f0c6, FUN_0052f0c6, "regs(eax:u32, esi:ptr[0x40]:u8{0,1,2,3,0x44,0x80,0x7f,0xff}) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=layout=tab_00835846+lim_0052f1f0")

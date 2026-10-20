#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sends the up to eight name characters at edi + 0x14 to the log routine 0x0054d3a1 (al = character;
// that routine restores every register and leaves the flags of its last compare/increment). The scan
// stops at the first byte below 0x20 (flags of that `cmp al, 0x20`) or after eight characters (flags
// of the last callee; `loop` writes no flag). esi ends one past the last byte read, ecx is the
// remaining count and al the last byte read.
// FUNCTION: GP4 0x0054377d
void __cdecl FUN_0054377d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = r->edi + 0x14u;
    c.ecx = 8u;
    for (;;) {
        const uint8_t ch = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
        c.esi = c.esi + 1u;
        c.eax = (c.eax & 0xffffff00u) | ch;
        if (ch < 0x20u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(ch, 0x20u));
            break;
        }
        gp4::call_regs(0x0054d3a1, c);
        c.ecx = c.ecx - 1u;
        if (c.ecx == 0u) {
            break;
        }
    }
    *r = c;
}
GP4_IMPL(0x0054377d, FUN_0054377d, "regs(eax:u32, ecx:u32, edi:ptr[0x40]:u8{=0x41,0x42,0x61,0x20,0x30,0x7a,0x05}) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_004fd730")

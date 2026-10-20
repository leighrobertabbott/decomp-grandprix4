#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the zero-terminated string at esi (read through cs, flat) one character at a time through
// 0x0051f3bd (al = character). eax and esi are restored (push/pop); the loop ends on `cmp al, 0`
// with al = 0, whose flags the pops keep. The other registers come from the last callee; esi is
// whatever the callee left plus the increment, since the loop re-reads it every round.
// FUNCTION: GP4 0x0051f4a9
void __cdecl FUN_0051f4a9(gp4::Regs* r) {
    const uint32_t eax0 = r->eax;
    const uint32_t esi0 = r->esi;
    gp4::Regs c = *r;
    for (;;) {
        const uint8_t ch = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
        c.esi = c.esi + 1u;
        c.eax = (c.eax & 0xffffff00u) | ch;
        if (ch == 0) {
            break;
        }
        gp4::call_regs(0x0051f3bd, c);
    }
    r->eax = eax0;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = esi0;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(0u, 0u));
}
GP4_IMPL(0x0051f4a9, FUN_0051f4a9, "regs(eax:u32, esi:ptr[0x40]:u8{=0,0,0,0x41,0x2d,0x39}, edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")

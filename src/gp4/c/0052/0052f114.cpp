#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x00826e3e is 0x80: draws string-table entry 0x4ba (esi is saved around that call),
// then prints the byte at [esi] as an unpadded decimal number (lodsb; movzx; bh = 0), the character
// '/' (0x2f) and the byte at [esi+1], and finally advances the text line counter through 0x0052f1f0.
// esi ends advanced by two; eax/ebx and the flags are whatever 0x0052f1f0 leaves, the other registers
// are untouched. Assumes DF clear for the lodsb. When the byte differs only the flags of
// `cmp byte [0x826e3e], 0x80` change.
// FUNCTION: GP4 0x0052f114
void __cdecl FUN_0052f114(gp4::Regs* r) {
    const uint32_t tag = GP4_GLOBAL(uint8_t, 0x00826e3e);
    const uint32_t cmpFlags = gp4::flags::sub8(tag, 0x80u);
    if (tag != 0x80u) {
        r->eflags = gp4::flags::merge(r->eflags, cmpFlags);
        return;
    }

    const uint32_t cursor = r->esi;
    const uint8_t* text = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, cmpFlags);

    c.esi = 0x4bau;
    gp4::call_regs(0x0054d1a2, c);

    c.esi = cursor + 1u;
    c.eax = text[0];
    c.ebx &= 0xffff00ffu;
    gp4::call_regs(0x0054d969, c);

    c.eax = (c.eax & 0xffffff00u) | 0x2fu;
    gp4::call_regs(0x0054d3a1, c);

    c.esi = cursor + 2u;
    c.eax = text[1];
    c.ebx &= 0xffff00ffu;
    gp4::call_regs(0x0054d969, c);

    gp4::call_regs(0x0052f1f0, c);
    *r = c;
}
GP4_IMPL(0x0052f114, FUN_0052f114, "regs(esi:ptr[0x10]:bytes) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846+lim_0052f1f0+log_004fd730+textout_0054d211")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054cc37
// Sets a colour (al = palette index) for the 2D renderer through 0x0054cc96. Flag byte
// [0x00833cd6] clear: the index is clamped to 0x18 and the 16-bit colour comes from the table
// 0x00838bd4. Otherwise, with [0x007ad869] set, it comes from the table 0x006e2568 (and, when
// [0x00ae6740] is set, is repacked: (c & 0x1f) | ((c & 0xffc0) >> 1), ebx = the high part);
// with [0x007ad869] clear the index itself goes to [0x00833cd5]. The colour is passed in eax
// to 0x0054cc96, which preserves every register; the flags are the callee's, or those of the
// `cmp byte [0x007ad869], 0` (equal) when it is skipped.
void __cdecl FUN_0054cc37(gp4::Regs* r) {
    uint32_t index = r->eax & 0xffu;
    uint32_t colour;
    uint32_t ebx = r->ebx;

    if (GP4_GLOBAL(uint8_t, 0x00833cd6) == 0u) {
        if (index > 0x19u) {
            index = 0x18u;
        }
        colour = GP4_ARRAY(uint16_t, 0x00838bd4)[index];
    } else {
        const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x007ad869);
        if (enabled == 0u) {
            GP4_GLOBAL(uint8_t, 0x00833cd5) = static_cast<uint8_t>(index);
            r->eax = index;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u));
            return;
        }
        colour = GP4_ARRAY(uint16_t, 0x006e2568)[index];
        if (GP4_GLOBAL(uint8_t, 0x00ae6740) != 0u) {
            ebx = colour & 0xffc0u;
            colour &= 0x1fu;
            ebx >>= 1;
            colour |= ebx;
        }
    }

    gp4::Regs c = *r;
    c.eax = colour;
    c.ebx = ebx;
    gp4::call_regs(0x0054cc96, c);

    r->eax = colour;
    r->ebx = ebx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054cc37, FUN_0054cc37, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=col_0054cc37")

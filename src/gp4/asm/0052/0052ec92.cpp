#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips the first word of the zero-terminated string at esi: esi ends at the first non-space character
// after the first space. When the string ends before that (no space, or only spaces after it) esi is
// restored to its entry value. eax and ebx are saved and restored. The flags are those of the last
// `cmp al, 0` (ZF = PF = 1 on the restore path) or, when a next word is found, of the `dec esi` that
// follows a `cmp al, 0` with a nonzero al (CF = 0).
// FUNCTION: GP4 0x0052ec92
void __cdecl FUN_0052ec92(gp4::Regs* r) {
    const uint32_t start = r->esi;
    uint32_t cursor = start;
    uint32_t ch;

    for (;;) {
        ch = GP4_GLOBAL(uint8_t, cursor);
        cursor += 1u;
        if (ch == 0u) {
            r->esi = start;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(ch, 0u));
            return;
        }
        if (ch == 0x20u) {
            break;
        }
    }

    do {
        ch = GP4_GLOBAL(uint8_t, cursor);
        cursor += 1u;
    } while (ch == 0x20u);

    if (ch == 0u) {
        r->esi = start;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(ch, 0u));
        return;
    }
    r->esi = cursor - 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(cursor, gp4::flags::sub8(ch, 0u)));
}
GP4_IMPL(0x0052ec92, FUN_0052ec92, "regs(esi:ptr[0x40]:u8{=0,0x20,0x41,0x42}) -> (esi, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Message handler keyed by edx (mirror of 0x0054f31b). Codes 2 and 3 step a counter stored in the field [ebx]
// (read and written at the width the esi record's byte +0xd selects, by 0x00539944 / 0x00539a13) down: it
// becomes value - 1, or the dword at esi+0x2a (the upper bound) when that is below the dword at esi+0x26
// (signed). Then the (register preserving) screen refresh 0x0054f54b runs and bit 6 of byte esi+0x1f is set;
// the flags are those of that final `or`. Code 1 only clears edx; any other code leaves everything alone. The
// flags of those two exits are the `cmp edx, 1` / `cmp edx, 3` that decided them.
// FUNCTION: GP4 0x0054f34e
void __cdecl FUN_0054f34e(gp4::Regs* r) {
    const uint32_t code = r->edx;
    if (code == 1u) {
        r->edx = 0;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(code, 1u));
        return;
    }
    if (code != 2u && code != 3u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(code, 3u));
        return;
    }

    gp4::Regs c = *r;
    gp4::call_regs(0x00539944, c);
    const uint32_t value = c.eax;
    c.eax = value - 1u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(value, c.eflags));
    const uint32_t lower = GP4_FIELD(uint32_t, c.esi, 0x26);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, lower));
    if (static_cast<int32_t>(c.eax) < static_cast<int32_t>(lower)) {
        c.eax = GP4_FIELD(uint32_t, c.esi, 0x2a);
    }
    gp4::call_regs(0x00539a13, c);
    gp4::call_regs(0x0054f54b, c);

    const uint32_t flagsByte = GP4_FIELD(uint8_t, c.esi, 0x1f) | 0x40u;
    GP4_FIELD(uint8_t, c.esi, 0x1f) = static_cast<uint8_t>(flagsByte);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(flagsByte));
    *r = c;
}
GP4_IMPL(0x0054f34e, FUN_0054f34e, "regs(edx:u32{2,3,1,0,4,5}, ebx:ptr[8]:bytes, esi:ptr[0x40]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")

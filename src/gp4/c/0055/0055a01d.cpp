#include <gp4/gp4.h>
#include <gp4/flags.h>

// Derives the interpolation words of the whole 0x160-byte ring [ [0x866ec0], [0x866eb8] ) inside a
// pushal/popal pair. The start record gets both halves (0x00559df7 = side 0, 0x00559e3a = side 1); for every
// following record, unless byte +0x48 has bit 0x20 (side 0) / bit 0x10 (side 1) set, the half is derived
// and, when the previous record of that side is not its immediate neighbour, the run between them is
// interpolated (0x00559e7d with ebx = run start for side 0, 0x00559f4d with esi = run start for side 1).
// The loop ends on `cmp edi, [0x866eb8]; jne`, so the flags are those of that final compare (zero).
// FUNCTION: GP4 0x0055a01d
void __cdecl FUN_0055a01d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.edi = GP4_GLOBAL(uint32_t, 0x00866ec0);
    gp4::call_regs(0x00559df7, c);
    c.ebx = c.edi;
    gp4::call_regs(0x00559e3a, c);
    c.esi = c.edi;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.edi, 0x160u));
    c.edi += 0x160u;

    for (;;) {
        const uint32_t bits_a = GP4_FIELD(uint8_t, c.edi, 0x48) & 0x20u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits_a));
        if (bits_a == 0u) {
            gp4::call_regs(0x00559df7, c);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.ebx, 0x160u));
            c.eax = c.ebx + 0x160u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, c.edi));
            if (c.eax != c.edi) {
                gp4::call_regs(0x00559e7d, c);
            }
            c.ebx = c.edi;
        }

        const uint32_t bits_b = GP4_FIELD(uint8_t, c.edi, 0x48) & 0x10u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits_b));
        if (bits_b == 0u) {
            gp4::call_regs(0x00559e3a, c);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.esi, 0x160u));
            c.eax = c.esi + 0x160u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, c.edi));
            if (c.eax != c.edi) {
                gp4::call_regs(0x00559f4d, c);
            }
            c.esi = c.edi;
        }

        c.edi += 0x160u;
        const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866eb8);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edi, end));
        if (c.edi == end) {
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0055a01d, FUN_0055a01d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=multiply_0040ce7b+lerp_00556791+ring_00555b28+geo_005558c6+bits_0055616f; wall=3600")

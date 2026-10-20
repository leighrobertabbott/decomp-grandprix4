#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: nothing when the width ebp - ebx is below 0x39 (unsigned). Otherwise the empty hook
// 0x0054a1d7 and the sorted-list insert 0x00549db9 run with ebp + 7 and ecx + 6, and unless bit 4 of
// the item byte [esi + 0x1f] is set, the empty hook 0x00531b39 gets edx = 0x10d with ebx/eax/ecx
// moved by 2 (1 when bit 7 of +0x1f is set) and ebp by 5 more, then the empty hook 0x00531b37 gets a
// style in edx: 0x101 when the dword [esi + 0x12] is 0x89f (else 0), + 3 when the item id +0xc equals
// the selection byte [0x0082fd45], + 6 when [0x007aafd3] == 0xff and [0x007ab1a0], [0x007aafe0] are
// not 0xff, and ebx/eax/ebp/ecx + 1 when bit 7 of +0x1f is set. popal restores every GPR; the flags
// are those of the last flag writer (cmp of the width, test of bit 4, test of bit 7 or inc ecx).
// FUNCTION: GP4 0x00539feb
void __cdecl FUN_00539feb(gp4::Regs* r) {
    const uint32_t span = r->ebp - r->ebx;
    uint32_t arith = gp4::flags::sub32(span, 0x39u);
    if (span < 0x39u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    const uint32_t item = r->esi;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edx = span;
    c.ebp += 7u;
    c.ecx += 6u;
    gp4::call_regs(0x0054a1d7, c);
    gp4::call_regs(0x00549db9, c);

    const uint32_t bits = GP4_FIELD(uint8_t, item, 0x1f);
    arith = gp4::flags::logic8(bits & 0x10u);
    if ((bits & 0x10u) == 0u) {
        const uint32_t step = (bits & 0x80u) != 0u ? 1u : 2u;
        gp4::Regs t = *r;
        t.st_in = 0;
        t.st_out = 0;
        t.edx = 0x10du;
        t.ebx += step;
        t.eax += step;
        t.ebp += 5u + step;
        t.ecx += step;
        gp4::call_regs(0x00531b39, t);

        uint32_t style = GP4_FIELD(uint32_t, item, 0x12) == 0x89fu ? 0x101u : 0u;
        if (GP4_FIELD(uint8_t, item, 0x0c) == GP4_GLOBAL(uint8_t, 0x0082fd45)) {
            style += 3u;
        }
        if (GP4_GLOBAL(uint8_t, 0x007aafd3) == 0xffu && GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu &&
            GP4_GLOBAL(uint8_t, 0x007aafe0) != 0xffu) {
            style += 6u;
        }

        gp4::Regs h = *r;
        h.st_in = 0;
        h.st_out = 0;
        h.edx = style;
        arith = gp4::flags::logic8(bits & 0x80u);
        if ((bits & 0x80u) != 0u) {
            arith = gp4::flags::inc32(h.ecx, arith);
            h.ebx += 1u;
            h.eax += 1u;
            h.ebp += 1u;
            h.ecx += 1u;
        }
        gp4::call_regs(0x00531b37, h);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00539feb, FUN_00539feb, "regs(esi:ptr[0x40]:u16{0x89f,0x89f,0x89f,0,0,0,0x1000,0x8000,0x9000,0xff}, eax:u32, ebx:u32, ebp:rel[ebx,1,0..0x80], ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9; cases=1000")

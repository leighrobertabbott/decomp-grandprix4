#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper around the empty hook 0x0054a1d7, the sorted-list insert 0x00549db9 (fed eax, ebx,
// ebp, ecx) and, when bit 4 of the byte [esi + 0x1f] is clear, the empty hook 0x00531b37 (which the
// original reaches with edx = 0xb0 + 9 if the three state bytes [0x7aafd3] == 0xff,
// [0x7ab1a0] != 0xff, [0x7aafe0] != 0xff, + 3 if [esi + 0xc] == [0x0082fd45], + 3 more if that
// and bit 7 of [esi + 0x1f]). popal restores every GPR but not the flags: they are those of the
// last flag writer on the path taken.
// FUNCTION: GP4 0x0053a0d3
void __cdecl FUN_0053a0d3(gp4::Regs* r) {
    const uint32_t record = r->esi;
    gp4::cdecl_<0x0054a1d7, void>();

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00549db9, c);

    const uint32_t bits = GP4_FIELD(uint8_t, record, 0x1f);
    uint32_t arith = gp4::flags::logic8(bits & 0x10u);
    if ((bits & 0x10u) == 0u) {
        uint32_t edx = 0xb0u;
        if (GP4_GLOBAL(uint8_t, 0x007aafd3) == 0xffu && GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu &&
            GP4_GLOBAL(uint8_t, 0x007aafe0) != 0xffu) {
            edx += 9u;
        }
        const uint32_t ah = GP4_FIELD(uint8_t, record, 0x0c);
        const uint32_t expected = GP4_GLOBAL(uint8_t, 0x0082fd45);
        arith = gp4::flags::sub8(ah, expected);
        if (ah == expected) {
            arith = gp4::flags::add32(edx, 3u);
            edx += 3u;
            arith = gp4::flags::logic8(bits & 0x80u);
            if ((bits & 0x80u) != 0u) {
                arith = gp4::flags::add32(edx, 3u);
                edx += 3u;
            }
        } else {
            arith &= ~gp4::flags::CF;
        }
        gp4::cdecl_<0x00531b37, void>();
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053a0d3, FUN_0053a0d3, "regs(esi:ptr[0x40]:u8{0,0x10,0x80,0xff}, eax:u32, ebx:u32, ebp:u32, ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")

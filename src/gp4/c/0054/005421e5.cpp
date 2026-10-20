#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bubble sort of the pointer table at 0x00830b0c (count in the dword at 0x00830b04), DESCENDING by
// the signed word at +0x0a of each pointed-to record, ties broken by the signed dword at +0x0c
// (a pair is swapped when word(a) < word(b), or equal words and dword(a) < dword(b)).
// pushal/popal wrapper: every GPR is restored; the flags are those of the last flag writer:
// "sub ecx,1" when the table has fewer than two entries, otherwise the final "cmp dl,0xff" of a
// pass that swapped nothing (dl = 0).
// FUNCTION: GP4 0x005421e5
void __cdecl FUN_005421e5(gp4::Regs* r) {
    int32_t* const table = GP4_ARRAY(int32_t, 0x00830b00);
    uint32_t arith;
    for (;;) {
        const int32_t count = table[1];
        arith = gp4::flags::sub32(static_cast<uint32_t>(count), 1u);
        if (count <= 1) {
            break;
        }
        bool swapped = false;
        int32_t* p = table + 3;
        for (uint32_t k = static_cast<uint32_t>(count) - 1u; k != 0u; --k, ++p) {
            const uint32_t a = static_cast<uint32_t>(p[0]);
            const uint32_t b = static_cast<uint32_t>(p[1]);
            const int16_t wa = GP4_FIELD(int16_t, a, 0x0a);
            const int16_t wb = GP4_FIELD(int16_t, b, 0x0a);
            if (wa > wb) {
                continue;
            }
            if (wa == wb && GP4_FIELD(int32_t, a, 0x0c) >= GP4_FIELD(int32_t, b, 0x0c)) {
                continue;
            }
            p[0] = static_cast<int32_t>(b);
            p[1] = static_cast<int32_t>(a);
            swapped = true;
        }
        if (!swapped) {
            arith = gp4::flags::sub8(0u, 0xffu);
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005421e5, FUN_005421e5, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=sort_00542224+sortkeys_005421e5")

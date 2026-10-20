#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00542257
// Bubble sort of the pointer table at 0x00830b0c (count = dword 0x00830b04) of the 0x20-byte
// datalog records, with the order key (record a before record b):
//   signed byte +7 ascending; on a tie signed word +0xa descending; on a further tie the original
//   compares the dword +0xc of `a` with itself (esi both times), so a full tie never swaps.
// A pass that swapped anything restarts from the top (dl == 0xff). pushal/popal restore every
// register; the flags are those of the last flag writer: `sub ecx, 1` when fewer than two entries
// remain (count <= 1, signed), otherwise the final `cmp dl, 0xff` of a pass without a swap (dl = 0).
void __cdecl FUN_00542257(gp4::Regs* r) {
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
            const int8_t aClass = GP4_FIELD(int8_t, a, 7);
            const int8_t bClass = GP4_FIELD(int8_t, b, 7);
            bool swap;
            if (aClass < bClass) {
                swap = false;
            } else if (aClass != bClass) {
                swap = true;
            } else {
                const int16_t aKey = GP4_FIELD(int16_t, a, 0xa);
                const int16_t bKey = GP4_FIELD(int16_t, b, 0xa);
                if (aKey > bKey) {
                    swap = false;
                } else if (aKey != bKey) {
                    swap = true;
                } else {
                    swap = false;
                }
            }
            if (swap) {
                p[0] = static_cast<int32_t>(b);
                p[1] = static_cast<int32_t>(a);
                swapped = true;
            }
        }
        if (!swapped) {
            arith = gp4::flags::sub8(0u, 0xffu);
            break;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00542257, FUN_00542257, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=sort_00542224+sortkeys_005421e5")

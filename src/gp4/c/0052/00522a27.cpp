#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bubble pass over the order list (0x00675958, [0x0066d8ec] bytes): while some adjacent pair has a
// non-negative byte followed by a negative (bit 7 set) one, the two are swapped, so the flagged
// entries drift to the front. pushal/popal wrapper: every GPR is restored, the flags are those of
// the last flag writer (popal does not restore them): "cmp ebp,2" for a count below 2 (signed),
// otherwise the closing "or edx,edx" of the final swap-free pass (edx = 0).
// FUNCTION: GP4 0x00522a27
void __cdecl FUN_00522a27(gp4::Regs* r) {
    const int32_t count = GP4_GLOBAL(int32_t, 0x0066d8ec);
    uint32_t arith;
    if (count < 2) {
        arith = gp4::flags::sub32(static_cast<uint32_t>(count), 2u);
    } else {
        uint8_t* const current = GP4_ARRAY(uint8_t, 0x00675958);
        uint8_t* const next = GP4_ARRAY(uint8_t, 0x00675959);
        int32_t swapped;
        do {
            swapped = 0;
            for (int32_t i = 0; i != count - 1; ++i) {
                if (static_cast<int8_t>(current[i]) >= 0 && static_cast<int8_t>(next[i]) < 0) {
                    const uint8_t low = current[i];
                    const uint8_t high = next[i];
                    next[i] = low;
                    current[i] = high;
                    swapped = -1;
                }
            }
        } while (swapped != 0);
        arith = gp4::flags::logic32(0u);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522a27, FUN_00522a27, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+list_00443278")

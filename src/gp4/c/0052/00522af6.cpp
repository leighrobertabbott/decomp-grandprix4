#include <gp4/gp4.h>
#include <gp4/flags.h>

// Recounts the flagged order-list bytes (0x00522a78), and when there are any walks the list
// 0x00675958 from index 0x16 up to the count [0x0066d8ec]: every entry with bit 7 set is moved
// into the front 22-byte window by 0x00522b2f (which removes the first unflagged byte of the
// window) and stored in its last slot (+0x15); a window without an unflagged byte (CF = 1) ends
// the walk. pushal/popal: no register changes; the flags are those of the last flag writer, the
// word-test of the count, the final `cmp esi, count`, or the failing 0x00522b2f.
// FUNCTION: GP4 0x00522af6
void __cdecl FUN_00522af6(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00522a78, c);

    const uint32_t flagged = GP4_GLOBAL(uint16_t, 0x007a93a8);
    uint32_t arith = gp4::flags::logic32(flagged);
    if (flagged != 0u) {
        uint8_t* const list = GP4_ARRAY(uint8_t, 0x00675958);
        for (int32_t index = 0x16;; ++index) {
            const int32_t count = GP4_GLOBAL(int32_t, 0x0066d8ec);
            arith = gp4::flags::sub32(static_cast<uint32_t>(index), static_cast<uint32_t>(count));
            if (index >= count) {
                break;
            }
            const uint8_t value = list[index];
            if (static_cast<int8_t>(value) < 0) {
                c.ebx = 0x00675958u;
                gp4::call_regs(0x00522b2f, c);
                if ((c.eflags & gp4::flags::CF) != 0u) {
                    arith = c.eflags;
                    break;
                }
                (list)[0x15] = value;
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522af6, FUN_00522af6, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=walk_00522af6")

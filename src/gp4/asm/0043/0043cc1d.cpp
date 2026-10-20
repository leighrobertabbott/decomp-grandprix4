#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043cc1d
void __cdecl FUN_0043cc1d(gp4::Regs* r) {
    // pushal ... popal restores every register; only the memory clears and the flags of the
    // last flag writer (cmp edx, 0x2c with edx == 0x2c: ZF = PF = 1, the rest clear) remain.
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    // The masked byte minus one is clamped above (signed) to 21 only: a zero byte gives row -1.
    int32_t row = static_cast<int32_t>(body[0x7c] & 0x3fu) - 1;
    if (row > 0x15) row = 0x15;
    const int32_t base = row * 0x2c;
    const uint8_t keep = GP4_ARRAY(const uint8_t, 0x0066d936)[row];
    for (int32_t column = 0; column != 0x2c; ++column) {
        if (static_cast<uint8_t>(column) != keep) {
            GP4_ARRAY(uint8_t, 0x0066d962)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066dd2a)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066e0f2)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066e4ba)[base + column] = 0;
            GP4_ARRAY(uint8_t, 0x0066e882)[base + column] = 0;
            GP4_ARRAY(uint32_t, 0x0066ec60)[base + column] = 0;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043cc1d, FUN_0043cc1d, "regs(esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=counter_0040f0dd")

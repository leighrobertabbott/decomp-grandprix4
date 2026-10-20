#include <gp4/gp4.h>
#include <gp4/flags.h>

// Index of the sort key of an order-list byte: ((b & 0x3f) - 1) * 4 as a 16-bit value, sign
// extended, added to the key table at 0x675970 (a byte with no low bits reads 0x67596c).
static inline int32_t OrderKey(uint8_t entry) {
    const uint16_t scaled = static_cast<uint16_t>(static_cast<uint16_t>((entry & 0x3fu) - 1u) << 2);
    return *reinterpret_cast<const int32_t*>(GP4_ARRAY(uint8_t, 0x00675970) +
                                             static_cast<int16_t>(scaled));
}

// pushal/popal bubble sort of the order list at 0x675958: [0x66d8ec] entries, adjacent bytes are
// swapped while the key of the later one is below the key of the earlier one (signed), until a
// whole pass makes no swap. The scratch globals 0x934df0 (key) and 0x934df4 (swap counter) are
// saved and restored; the registers come back unchanged and the flags are those of the final
// "test word [0x934df4], 0xffff" with a zero counter.
// FUNCTION: GP4 0x0052286e
void __cdecl FUN_0052286e(gp4::Regs* r) {
    const uint32_t savedKey = GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t savedCount = GP4_GLOBAL(uint32_t, 0x00934df4);
    uint8_t* list = GP4_ARRAY(uint8_t, 0x00675958);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x0066d8ec) - 1u;
    do {
        GP4_GLOBAL(uint16_t, 0x00934df4) = 0;
        uint32_t i = 0;
        do {
            GP4_GLOBAL(int32_t, 0x00934df0) = OrderKey(list[i]);
            if (OrderKey(list[i + 1]) < GP4_GLOBAL(int32_t, 0x00934df0)) {
                const uint8_t held = list[i];
                list[i] = list[i + 1];
                list[i + 1] = held;
                GP4_GLOBAL(uint16_t, 0x00934df4) = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00934df4) + 1u);
            }
            ++i;
        } while (i != last);
    } while (GP4_GLOBAL(uint16_t, 0x00934df4) != 0);
    GP4_GLOBAL(uint32_t, 0x00934df4) = savedCount;
    GP4_GLOBAL(uint32_t, 0x00934df0) = savedKey;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(0u));
}
GP4_IMPL(0x0052286e, FUN_0052286e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+keys_00675970")

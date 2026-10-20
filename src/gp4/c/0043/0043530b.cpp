#include <gp4/gp4.h>

// Car record at a byte offset of the 22-record array at 0x66fef0 (the offsets come from the
// order list 0x66fe3c). Kept out of line so the record fields stay relative to the record.
static __declspec(noinline) uint8_t* carRecord(int32_t offset) {
    return GP4_ARRAY(uint8_t, 0x0066fef0) + offset;
}

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Walks the order list backwards from the entry before the car [0x679d30] (cars ahead of it),
// skipping cars with a zero dword +0x240, flag +0x7a bit 7, or a +0x8d bit 1 different from the
// car's; stops at the list guard or at the first car more than 0x20 ahead in word +0x68. For each
// remaining car it takes the gap (word +0xe2 difference, or 0x40dc8a when either +0x9c has bit 1;
// the first such car records itself in 0x62256c and the doubled gap as the maximum in 0x622570),
// clamps (gap - 0x40) to 0..0x800 and the lateral offset |(+0x90 - +0x94) difference| minus that
// to 0..0x200, looks up the Q14 table 0x7c5c5c at 4 * that offset, and scales
// (0x800 - clamped gap) * table >> 17 by min(word +0x9a, 0x800) << 3 >> 14 and by word +0xba >> 14;
// 0x62255c keeps the maximum (starting at 0x622564). Finally 0x622568 = clamp(0x62255c *
// 0x622560 >> 14, 0, 0x4000). pushal/popal keep every register; the flags are those of the final
// "cmp eax, 0" on the value already clamped to at most 0x4000.
// FUNCTION: GP4 0x0043530b
void __cdecl FUN_0043530b(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x00622568) = 0;
    GP4_GLOBAL(int32_t, 0x00622570) = 0;
    GP4_GLOBAL(int32_t, 0x0062255c) = GP4_GLOBAL(int32_t, 0x00622564);
    GP4_GLOBAL(uint32_t, 0x0062256c) = 0;

    uint8_t* player = GP4_GLOBAL(uint8_t*, 0x00679d30);
    const int16_t* entry =
        reinterpret_cast<const int16_t*>(GP4_ARRAY(uint8_t, 0x0066fe3c) + GP4_FIELD(uint8_t, player, 0x7d));
    for (;;) {
        --entry;
        const int16_t offset = *entry;
        if (offset < 0) {
            break;
        }
        uint8_t* car = carRecord(offset);
        if (GP4_FIELD(uint32_t, car, 0x240) == 0u) {
            continue;
        }
        if ((GP4_FIELD(uint8_t, car, 0x7a) & 0x80u) != 0u) {
            continue;
        }
        const bool playerBit = (GP4_FIELD(uint8_t, player, 0x8d) & 2u) != 0u;
        const bool carBit = (GP4_FIELD(uint8_t, car, 0x8d) & 2u) != 0u;
        if (playerBit != carBit) {
            continue;
        }
        const uint16_t ahead =
            static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x68) - GP4_FIELD(uint16_t, player, 0x68));
        if (ahead > 0x20u) {
            break;
        }

        uint16_t gap;
        if ((GP4_FIELD(uint8_t, player, 0x9c) & 2u) != 0u || (GP4_FIELD(uint8_t, car, 0x9c) & 2u) != 0u) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.eax = ahead;
            call.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(player));
            call.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
            gp4::call_regs(0x0040dc8a, call);
            gap = static_cast<uint16_t>(call.eax);
        } else {
            gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0xe2) - GP4_FIELD(uint16_t, player, 0xe2));
        }

        if (GP4_GLOBAL(uint32_t, 0x0062256c) == 0u) {
            GP4_GLOBAL(uint32_t, 0x0062256c) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
            const int32_t doubled = static_cast<int32_t>(static_cast<int16_t>(gap)) * 2;
            if (GP4_GLOBAL(int32_t, 0x00622570) < doubled) {
                GP4_GLOBAL(int32_t, 0x00622570) = doubled;
            }
            gap = static_cast<uint16_t>(doubled);
        }

        uint16_t lateral16 = static_cast<uint16_t>(GP4_FIELD(uint16_t, player, 0x90) - GP4_FIELD(uint16_t, player, 0x94)
            - GP4_FIELD(uint16_t, car, 0x90) + GP4_FIELD(uint16_t, car, 0x94));
        if ((lateral16 & 0x8000u) != 0u) {
            lateral16 = static_cast<uint16_t>(0u - lateral16);
        }
        const int32_t lateral = static_cast<int16_t>(lateral16);

        int32_t overlap = static_cast<int32_t>(static_cast<int16_t>(gap)) - 0x40;
        if (overlap < 0) {
            overlap = 0;
        }
        if (overlap > 0x800) {
            overlap = 0x800;
        }
        int32_t excess = lateral - overlap;
        if (excess < 0) {
            excess = 0;
        }
        if (excess > 0x200) {
            excess = 0x200;
        }
        // excess <= 0x200, so excess << 5 <= 0x4000 and the word is never negative.
        const uint32_t index = ((static_cast<uint32_t>(excess) << 5) >> 2) & 0xfffeu;
        const int32_t table = GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), index);

        const int64_t p0 = static_cast<int64_t>(0x800 - overlap) * static_cast<int64_t>(table);
        const int32_t a = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(p0) >> 17));
        int32_t width = GP4_FIELD(int16_t, car, 0x9a);
        if (width > 0x800) {
            width = 0x800;
        }
        const int64_t p1 = static_cast<int64_t>(a) * static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(width) << 3));
        const int32_t b = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14));
        const int64_t p2 = static_cast<int64_t>(static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0xba))) * static_cast<int64_t>(b);
        const int32_t c = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(p2) >> 14));
        if (GP4_GLOBAL(int32_t, 0x0062255c) < c) {
            GP4_GLOBAL(int32_t, 0x0062255c) = c;
        }
    }

    const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062255c))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00622560));
    int32_t level = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    if (level > 0x4000) {
        level = 0x4000;
    }
    // cmp eax, 0: CF = OF = AF = 0.
    const uint32_t flags = parityFlag(static_cast<uint32_t>(level))
        | (level == 0 ? 0x40u : 0u)
        | (level < 0 ? 0x80u : 0u);
    if (level < 0) {
        level = 0;
    }
    GP4_GLOBAL(int32_t, 0x00622568) = level;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043530b, FUN_0043530b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+list_0066fe3c+gap_0043530b")

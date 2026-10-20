#include <gp4/gp4.h>

#pragma pack(push, 1)
// The first fields of a 0x160-byte track-segment record at 0x008943a0.
struct SegmentRecord {
    uint8_t Unknown00[4];
    uint16_t OriginX;
    uint8_t Unknown06[2];
    uint16_t OriginY;
};
#pragma pack(pop)

// Surface class around the track point of record 0: samples a 5 x 5 grid of cells (spacing 0xa0000 in
// 16.16 units, starting 0x140000 before the origin point) through 0x0040c536, whose class byte is added
// to a running sum (the sum is also passed to the lookup as its high selector word). The mean class
// (sum / 25, clamped to 0..4) is returned in eax and [0x00937de0] becomes 1 when it is at least 3.
// All registers are saved and restored by the original.
// FUNCTION: GP4 0x0051fbc5
int __cdecl FUN_0051fbc5() {
    const SegmentRecord* record = reinterpret_cast<const SegmentRecord*>(static_cast<uintptr_t>(0x008943a0));
    uint32_t x = record->OriginX;
    uint32_t y = static_cast<uint32_t>(record->OriginY) - 0x140000u;
    uint32_t sum = 0u;
    for (uint32_t row = 5u; row != 0u; --row) {
        y += 0xa0000u;
        x -= 0x140000u;
        for (uint32_t column = 5u; column != 0u; --column) {
            gp4::Regs c = {};
            c.eax = x;
            c.ebx = sum;
            c.edx = y;
            gp4::call_regs(0x0040c536, c);
            sum += c.eax;
            x += 0xa0000u;
        }
    }

    uint32_t mean = sum;
    if (sum != 0u) {
        uint32_t remainder;
        mean = gp4::div64(static_cast<uint64_t>(sum), 0x19u, &remainder);
    }
    int32_t clamped = static_cast<int32_t>(mean);
    if (clamped < 0) {
        clamped = 0;
    }
    if (clamped > 4) {
        clamped = 4;
    }
    GP4_GLOBAL(uint8_t, 0x00937de0) = 0u;
    if (static_cast<uint32_t>(clamped) >= 3u) {
        GP4_GLOBAL(uint8_t, 0x00937de0) = 1u;
    }
    return clamped;
}
GP4_IMPL(0x0051fbc5, FUN_0051fbc5, "cdecl() -> i32; globals=fuzz:i32+layout=map_0040c43b+geom_008943a0")

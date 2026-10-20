#include <gp4/gp4.h>

struct Unknown_0040c204_Segment {
    uint8_t Unknown00[4];
    int16_t Unknown04;
    uint8_t Unknown06[2];
    int16_t Unknown08;
    uint8_t Unknown0A[0x70];
    uint16_t Unknown7A;
};

// pushal; warm-up of the track surface: steps the simulation 0x40c1d6 / 0x40c490 until the
// accumulated [0x603854] steps reach 0x2580000, then looks up the surface class around the
// ring record [0x866ecc] - 0x1080 (record 0x7a selects a 10-byte entry at 0x675a82; its high
// nibble clamped to 0..11 plus the map value of 0x40c43b minus 2, clamped to 0..3, index the byte
// table 0x622658, clamped to 0..8) and stores it at 0x7a8ecf, then the byte of 0x51fbc5 at
// 0x7a8ed0. [0x62ac99] is 0xff while running. popal restores every register; the flags that
// escape are those left by 0x51fbc5.
// FUNCTION: GP4 0x0040c204
void __cdecl FUN_0040c204(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x0062ac99) = 0xff;
    int32_t elapsed = 0;
    do {
        gp4::call_regs(0x0040c1d6, c);
        gp4::call_regs(0x0040c490, c);
        elapsed = static_cast<int32_t>(static_cast<uint32_t>(elapsed) + GP4_GLOBAL(uint32_t, 0x00603854));
    } while (elapsed < 0x2580000);

    const Unknown_0040c204_Segment* segment = reinterpret_cast<const Unknown_0040c204_Segment*>(
        static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x1080u));
    const uint32_t entry = 0x00675a82u + static_cast<uint32_t>(segment->Unknown7A) * 10u;
    int32_t row = static_cast<int32_t>(GP4_GLOBAL(uint8_t, entry + 1) >> 4);
    if (row < 0) {
        row = 0;
    }
    if (row > 0xb) {
        row = 0xb;
    }

    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment));
    c.ebx = entry;
    c.ecx = static_cast<uint32_t>(row);
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(segment->Unknown04) >> 3);
    c.edx = static_cast<uint32_t>(static_cast<int32_t>(segment->Unknown08) >> 3);
    gp4::call_regs(0x0040c43b, c);

    int32_t column = static_cast<int32_t>(c.eax) - 2;
    if (column < 0) {
        column = 0;
    }
    int32_t index = (column & 3) | static_cast<int32_t>(c.ecx << 2);
    int32_t value = static_cast<int32_t>(GP4_ARRAY(const uint8_t, 0x00622658)[index]);
    if (value < 0) {
        value = 0;
    }
    if (value > 8) {
        value = 8;
    }
    GP4_GLOBAL(uint8_t, 0x007a8ecf) = static_cast<uint8_t>(value);

    c.eax = static_cast<uint32_t>(value);
    gp4::call_regs(0x0051fbc5, c);
    GP4_GLOBAL(uint8_t, 0x007a8ed0) = static_cast<uint8_t>(c.eax);
    GP4_GLOBAL(uint8_t, 0x0062ac99) = 0;

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0040c204, FUN_0040c204, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+map_0040c43b+ring_0040c0ae+state_0040bec7+state_0040bce6+twopass_0040c204")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine. ebx is a record whose word +0x7a selects a 10-byte entry of the table at
// 0x00675a82. The two dwords 0x00948b60/64 are copied to 0x00948b70/74, then the six bytes
// 0x00948b75 down to 0x00948b70 get bits 5..7 ORed in: level k (0..5) is clamp(hi - k, lo, 6) << 5,
// lo = entry[0] >> 4, hi = min(entry[1] >> 4, [0x00937dcc]) (signed). The byte at 0x00948b5d is
// 5 - min(entry[4] >> 4, table 0x00937dd0[entry[1] >> 4]), or 5 - [0x009328d8] when that dword is
// not negative; 0x00948b5e = (entry[3] & 0x70) << 1. Every register is restored; the flags are those
// of the final 8-bit `shl al, 1` (CF = AF = 0 since al <= 0x70, OF = bit 7 of the result).
// FUNCTION: GP4 0x00550be1
void __cdecl FUN_00550be1(gp4::Regs* r) {
    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t index = *reinterpret_cast<const uint16_t*>(record + 0x7a);
    const uint8_t* const entry = GP4_ARRAY(const uint8_t, 0x00675a82) + index * 10u;

    GP4_GLOBAL(uint32_t, 0x00948b70) = GP4_GLOBAL(uint32_t, 0x00948b60);
    GP4_GLOBAL(uint32_t, 0x00948b74) = GP4_GLOBAL(uint32_t, 0x00948b64);

    const int32_t lo = entry[0] >> 4;
    int32_t hi = entry[1] >> 4;
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00937dcc);
    if (hi > limit) {
        hi = limit;
    }
    uint8_t* const levels = GP4_ARRAY(uint8_t, 0x00948b70);
    for (int32_t k = 0; k < 6; ++k) {
        int32_t level = hi - k;
        if (level < lo) {
            level = lo;
        }
        if (level > 6) {
            level = 6;
        }
        levels[5 - k] |= static_cast<uint8_t>(static_cast<uint32_t>(level) << 5);
    }

    const uint32_t cap = GP4_ARRAY(const uint8_t, 0x00937dd0)[entry[1] >> 4];
    uint32_t value = entry[4] >> 4;
    if (static_cast<int32_t>(value) > static_cast<int32_t>(cap)) {
        value = cap;
    }
    const int32_t forced = GP4_GLOBAL(int32_t, 0x009328d8);
    if (forced >= 0) {
        value = static_cast<uint32_t>(forced);
    }
    GP4_GLOBAL(uint8_t, 0x00948b5d) = static_cast<uint8_t>(5u - value);

    const uint8_t shifted = static_cast<uint8_t>((entry[3] & 0x70u) << 1);
    GP4_GLOBAL(uint8_t, 0x00948b5e) = shifted;

    uint32_t flags = gp4::flags::logic8(shifted);
    if ((shifted & 0x80u) != 0u) {
        flags |= gp4::flags::OF;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00550be1, FUN_00550be1, "regs(ebx:ptr[0x82]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

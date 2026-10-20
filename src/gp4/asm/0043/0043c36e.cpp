#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Calls the table blend 0x0043c31b with eax = value; returns its eax.
static inline uint32_t blend(const gp4::Regs* r, uint32_t value) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = value;
    gp4::call_regs(0x0043c31b, call);
    return call.eax;
}

struct Unknown_0043c36e_Body {
    uint8_t Unknown00[0x70];
    uint8_t Unknown70;
    uint8_t Unknown71[0x136 - 0x71];
    uint8_t Unknown136;
    uint8_t Unknown137[0x198 - 0x137];
    uint8_t Unknown198[0x100];
};

// FUNCTION: GP4 0x0043c36e
// esi = body. pushal / popal wrapper: all registers are preserved, the result is CF (clear = the
// blended value 0x0043c31b is below 14000 + [0x0062d240] scaled; the same compare also leaves
// PF/AF/ZF/SF/OF of its `cmp`). Writes the dword 0x006227f0 (0, or [0x0062d238] * [0x006227f4] >> 14
// when byte [0x006227f8] is 0xff and the signed byte +0x136 is below the signed byte +0x198).
void __cdecl FUN_0043c36e(gp4::Regs* r) {
    const Unknown_0043c36e_Body* body = reinterpret_cast<const Unknown_0043c36e_Body*>(static_cast<uintptr_t>(r->esi));
    GP4_GLOBAL(uint32_t, 0x006227f0) = 0u;

    uint32_t flags;
    bool carry;
    if (static_cast<int8_t>(body->Unknown136) < static_cast<int8_t>(body->Unknown198[0])) {
        if (GP4_GLOBAL(uint8_t, 0x006227f8) == 0xffu) {
            const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062d238)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006227f4));
            GP4_GLOBAL(uint32_t, 0x006227f0) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        }
        const uint32_t index = body->Unknown136;
        const uint32_t current = body->Unknown198[1 + index];
        int32_t excess = static_cast<int32_t>(current) - static_cast<int32_t>(body->Unknown70);
        if (excess < 0) {
            excess = 0;
        }
        int32_t span = static_cast<int32_t>(current);
        if (index != 0u) {
            span -= static_cast<int32_t>(body->Unknown198[index]);
        }
        if (span == 0) {
            span = 1;
        }
        int32_t ratio = static_cast<int32_t>(static_cast<uint32_t>(excess) << 14) / span;
        if (ratio > 0x4000) {
            ratio = 0x4000;
        }
        if (ratio < 0) {
            ratio = 0;
        }
        flags = cmpFlags32(static_cast<uint32_t>(excess), 3u);
        carry = true;
        if (excess >= 3) {
            const uint32_t blended = blend(r, static_cast<uint32_t>(excess));
            const uint32_t base = GP4_GLOBAL(uint32_t, 0x0062d240) + 0x1770u + 0x1f40u;
            const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(base)) * static_cast<int64_t>(ratio);
            const uint32_t limit = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
            flags = cmpFlags32(blended, limit);
            carry = !(static_cast<int32_t>(blended) < static_cast<int32_t>(limit));
        }
    } else {
        const uint32_t start = (0u - static_cast<uint32_t>(body->Unknown70)) + GP4_GLOBAL(uint16_t, 0x007c2e96);
        const uint32_t blended = blend(r, start);
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x0062d240) + 0x1770u + 0x1f40u;
        flags = cmpFlags32(blended, limit);
        carry = !(static_cast<int32_t>(blended) < static_cast<int32_t>(limit));
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~0x1u) | (carry ? 1u : 0u);
}
GP4_IMPL(0x0043c36e, FUN_0043c36e, "regs(esi:ptr[0x2a0]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c31b")

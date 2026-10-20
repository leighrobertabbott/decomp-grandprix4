#include <gp4/gp4.h>

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (lhs < rhs ? 1u : 0u) | ((~parity & 1u) << 2)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00437888
void __cdecl FUN_00437888(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x0066fef0);
    int32_t count = 0;
    for (int i = 0; i < 22; ++i, rec += 0x41c) {
        if (GP4_FIELD(uint32_t, rec, 0x40) == 0) continue;
        if (rec == self) continue;
        if (rec == GP4_GLOBAL(uint8_t*, 0x0066d820)) continue;
        if (GP4_GLOBAL(uint8_t, 0x007c2e6c) != 0 && rec == GP4_GLOBAL(uint8_t*, 0x0066d82c)) continue;
        if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0 && rec == GP4_GLOBAL(uint8_t*, 0x0066d828)) continue;
        int16_t diff = static_cast<int16_t>(GP4_FIELD(uint16_t, rec, 0x68) - GP4_FIELD(uint16_t, self, 0x68));
        if (diff < 0) diff = static_cast<int16_t>(-diff);
        if (diff >= GP4_GLOBAL(int16_t, 0x00629bdc)) continue;
        ++count;
    }
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x0062a7c8);
    const uint32_t result = static_cast<uint32_t>(count) - limit;
    // CMP count, limit; POPAL keeps the flags; then CLC / STC replaces CF with (count >= limit signed)
    uint32_t flags = SubtractFlags(static_cast<uint32_t>(count), limit, result) & ~1u;
    if (count >= static_cast<int32_t>(limit)) flags |= 1u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00437888, FUN_00437888, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00437888")

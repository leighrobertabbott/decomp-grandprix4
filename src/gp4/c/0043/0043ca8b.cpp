#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Position (0x100..0x500, 24.8 fixed point) of the six-entry dword table: the index of its
// largest entry (the last one that is >= the running maximum, which starts at 0) is the stage that
// 0x0043c16a interpolates around.
static __declspec(noinline) uint32_t interpolateTable(const gp4::Regs* entry, const int32_t* table) {
    int32_t best = 0;
    uint32_t stage = 0;
    for (uint32_t i = 0; i != 6u; ++i) {
        if (table[i] >= best) {
            best = table[i];
            stage = i;
        }
    }
    gp4::Regs call = *entry;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = stage;
    call.ebp = static_cast<uint32_t>(best);
    call.ecx = 6u;
    call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    gp4::call_regs(0x0043c16a, call);
    return call.eax;
}

// (position + 0x80) >> 8, capped at 5; `flags` receives the flags of the `cmp eax, 5`.
static inline uint32_t band(uint32_t position, uint32_t* flags) {
    uint32_t value = (position + 0x80u) >> 8;
    *flags = cmpFlags32(value, 5u);
    if (static_cast<int32_t>(value) > 5) {
        value = 5u;
    }
    return value;
}

// The four blends, one call each (kept apart so every callee stays a direct call).
static __declspec(noinline) void blendHighHigh(gp4::Regs& call) { gp4::call_regs(0x0043c9e5, call); }
static __declspec(noinline) void blendHighLow(gp4::Regs& call) { gp4::call_regs(0x0043ca01, call); }
static __declspec(noinline) void blendLowHigh(gp4::Regs& call) { gp4::call_regs(0x0043ca4a, call); }
static __declspec(noinline) void blendLowLow(gp4::Regs& call) { gp4::call_regs(0x0043c9c9, call); }

// FUNCTION: GP4 0x0043ca8b
// Computes the gear bands of the two six-gear tables: 0x0043c1ba refreshes the table at 0x006227c4,
// both tables are interpolated by 0x0043c16a into [0x006227e0] / [0x006227dc] and their bands
// (0..5) stored as bytes at 0x0063b85e / 0x0063b85f. The two bands select the blend of the
// positions (0x0043c9e5 when both are >= 2, 0x0043ca01 when only the first is, 0x0043ca4a when
// only the second is, else 0x0043c9c9); the blend, floored at 0, goes to [0x006227e4] and, snapped
// by 0x0043c953, gives the band byte at 0x0063b861. ebx, ecx, edx, ebp, esi and edi are restored;
// eax is that last band and the flags are those of its `cmp eax, 5`.
void __cdecl FUN_0043ca8b(gp4::Regs* r) {
    gp4::Regs refresh = *r;
    refresh.st_in = 0;
    refresh.st_out = 0;
    gp4::call_regs(0x0043c1ba, refresh);

    uint32_t flags;
    const uint32_t first = interpolateTable(r, GP4_ARRAY(int32_t, 0x006227c4));
    GP4_GLOBAL(uint32_t, 0x006227e0) = first;
    GP4_GLOBAL(uint8_t, 0x0063b85e) = static_cast<uint8_t>(band(first, &flags));

    const uint32_t second = interpolateTable(r, GP4_ARRAY(int32_t, 0x0063b844));
    GP4_GLOBAL(uint32_t, 0x006227dc) = second;
    GP4_GLOBAL(uint8_t, 0x0063b85f) = static_cast<uint8_t>(band(second, &flags));

    const int8_t bandFirst = GP4_GLOBAL(int8_t, 0x0063b85e);
    const int8_t bandSecond = GP4_GLOBAL(int8_t, 0x0063b85f);

    gp4::Regs blend = *r;
    blend.st_in = 0;
    blend.st_out = 0;
    if (bandFirst >= 2) {
        if (bandSecond >= 2) {
            blendHighHigh(blend);
        } else {
            blendHighLow(blend);
        }
    } else {
        if (bandSecond >= 2) {
            blendLowHigh(blend);
        } else {
            blendLowLow(blend);
        }
    }
    int32_t position = static_cast<int32_t>(blend.eax);
    if (position < 0) {
        position = 0;
    }
    GP4_GLOBAL(int32_t, 0x006227e4) = position;

    gp4::Regs snap = *r;
    snap.st_in = 0;
    snap.st_out = 0;
    snap.eax = static_cast<uint32_t>(position);
    gp4::call_regs(0x0043c953, snap);

    const uint32_t last = band(snap.eax, &flags);
    GP4_GLOBAL(uint8_t, 0x0063b861) = static_cast<uint8_t>(last);

    r->eax = last;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ca8b, FUN_0043ca8b, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043c1ba+state_0043c31b+state_0043ca8b")

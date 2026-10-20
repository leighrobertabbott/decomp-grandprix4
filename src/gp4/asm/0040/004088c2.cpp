#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left - right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x004088c2
void __cdecl FUN_004088c2(gp4::Regs* r) {
    // pushal ... popal restores every register; only the byte table at esi + 0x199 and the flags
    // of the last flag writer remain.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t multiplier = static_cast<int32_t>(r->eax);
    const int32_t count = GP4_GLOBAL(int32_t, 0x00601fbc);
    uint32_t flags;

    if (count == 0) {
        // test dword [0x601fbc], -1 with zero: ZF = PF = 1, the rest clear
        flags = 0x44u;
    } else {
        const int32_t start = GP4_GLOBAL(int32_t, 0x00601fb8);
        int32_t index = start;
        uint32_t remaining = static_cast<uint32_t>(count);
        for (;;) {
            // remaining span of the window, clamped to 0..count
            int32_t span = static_cast<int32_t>(static_cast<uint32_t>(count) + static_cast<uint32_t>(start) - static_cast<uint32_t>(index));
            if (span < 0) span = 0;
            if (span > count) span = count;
            int32_t remainder;
            const int32_t quotient = gp4::idiv64(static_cast<int64_t>(span) * static_cast<int64_t>(multiplier), GP4_GLOBAL(int32_t, 0x00601fc0), &remainder);
            int32_t value = static_cast<int32_t>(static_cast<uint32_t>(quotient) + GP4_FIELD(uint8_t, body, 0x199u + static_cast<uint32_t>(index)));
            if (value < 0) value = 0;
            if (value > 0x64) value = 0x64;
            if (index >= 3) {
                // cmp edi, 3 ; jge
                flags = subFlags32(static_cast<uint32_t>(index), 3u);
                break;
            }
            GP4_FIELD(uint8_t, body, 0x199u + static_cast<uint32_t>(index)) = static_cast<uint8_t>(value);
            const uint32_t before = static_cast<uint32_t>(index);
            ++index;
            --remaining;
            if (remaining == 0u) {
                // inc edi keeps the carry of the cmp edi, 3 (edi < 3 here) and rewrites the rest
                const uint32_t after = before + 1u;
                flags = 1u | parityFlag(after) | (((before & 0xfu) == 0xfu) ? 0x10u : 0u)
                    | (after == 0u ? 0x40u : 0u) | ((after >> 24) & 0x80u)
                    | (after == 0x80000000u ? 0x800u : 0u);
                if (before >= 3u) flags &= ~1u;
                break;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004088c2, FUN_004088c2, "regs(eax:i32[-100000..100000], esi:ptr[0x1a0]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_004088c2")

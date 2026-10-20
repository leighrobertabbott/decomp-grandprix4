#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042efee
// Scales the magnitude of a per-slot value (record at 0x626fb8 + 0x40 * index, dword +0x20)
// minus a dead zone (0x603e80) by a gain (0x603e7c, 14-bit fraction), clamped to
// [0x603e78, 0x4000]. Returns the lower bound 0x603e78 unchanged when bit 0x80 of the
// byte at +0x51 of the object pointed to by [esi+0x1c] is clear. edx/edi/ebp are preserved;
// the flags left behind are those of the final "cmp eax, [0x603e78]" (or of the "test").
void __cdecl FUN_0042efee(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* owner = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, self, 0x1c)));
    const int32_t lowest = GP4_GLOBAL(int32_t, 0x00603e78);
    uint32_t flags;
    int32_t result;

    if ((owner[0x51] & 0x80u) == 0u) {
        // test byte [edi+0x51], 0x80 -> zero result: ZF = PF = 1, CF = OF = AF = SF = 0
        result = lowest;
        flags = 0x44u;
    } else {
        uint8_t* slot = GP4_ARRAY(uint8_t, 0x00626fb8) + static_cast<uint32_t>(owner[0xb6]) * 0x40u;
        int32_t value = GP4_FIELD(int32_t, slot, 0x20);
        if (value < 0) {
            value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
        }
        value = static_cast<int32_t>(static_cast<uint32_t>(value) - GP4_GLOBAL(uint32_t, 0x00603e80));
        if (value < 0) {
            value = 0;
        }
        const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603e7c));
        int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
        if (scaled > 0x4000) {
            scaled = 0x4000;
        }
        flags = subtractionFlags32(static_cast<uint32_t>(scaled), static_cast<uint32_t>(lowest));
        result = (scaled < lowest) ? lowest : scaled;
    }

    r->eax = static_cast<uint32_t>(result);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042efee, FUN_0042efee, "regs(esi:ptr[0x40]:layout=rec_0042efee) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0042efee")

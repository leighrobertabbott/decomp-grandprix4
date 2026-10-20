#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of SHRD r32, r32, count (count > 1) in the pinned oracle model.
static inline uint32_t shrdFlags32(uint32_t low, uint32_t high, uint32_t count) {
    const uint64_t joined = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t result = static_cast<uint32_t>(joined >> count);
    const uint32_t previous = static_cast<uint32_t>(joined >> (count - 1));
    return (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042640e
// eax = car-like record pointer, esi = second record. Returns in eax a clamped Q14 value derived
// from the table entry at 0x00626fb8 + index * 0x40 (+0x10) with index = byte [eax+0x134], else
// byte [eax+0xb6] when bit 7 of [eax+0x51] is set, else 0. ebx, edx and edi are preserved; the
// flags are those of the last flag writer (or/test/shrd).
void __cdecl FUN_0042640e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t index = GP4_FIELD(uint8_t, record, 0x134);
    uint32_t flags;
    uint32_t result = 0;

    if (index == 0) {
        index = GP4_FIELD(uint8_t, record, 0xb6);
        if (index == 0) {
            r->eax = 0;
            r->eflags = (r->eflags & ~0x8d5u) | testFlags8(0);
            return;
        }
        const uint32_t gate = GP4_FIELD(uint8_t, record, 0x51) & 0x80u;
        if (gate == 0) {
            r->eax = 0;
            r->eflags = (r->eflags & ~0x8d5u) | testFlags8(gate);
            return;
        }
    }

    int32_t value = GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x00626fb8), index * 0x40 + 0x10);
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }
    if (value > 0x4000) {
        const int32_t excess = static_cast<int32_t>(0u - (static_cast<uint32_t>(value) - 0x4000u));
        const int64_t knee = static_cast<int64_t>(excess) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00627ae0));
        value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(knee) >> 14) + 0x4000u);
    }
    const int64_t scaled = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00627ae4));
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);

    eax += static_cast<uint32_t>(GP4_FIELD(uint16_t, other, 0x100)) * 3u;
    eax = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 2);
    eax -= 0xf1cu;
    const int64_t wide = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(0x2d000);
    int32_t clamped = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(wide) >> 14));
    if (clamped < 0) {
        clamped = 0;
    }
    if (clamped > 0xe000) {
        clamped = 0xe000;
    }

    const uint32_t bit = GP4_FIELD(uint8_t, other, 0xe5) & 1u;
    if (bit == 0) {
        result = static_cast<uint32_t>(clamped);
        flags = testFlags8(bit);
    } else {
        const int64_t last = static_cast<int64_t>(clamped) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00627ae8));
        const uint32_t low = static_cast<uint32_t>(last);
        const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(last) >> 32);
        result = static_cast<uint32_t>(static_cast<uint64_t>(last) >> 14);
        flags = shrdFlags32(low, high, 14);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042640e, FUN_0042640e, "regs(eax:ptr[0x13c]:bytes, esi:ptr[0x10a]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

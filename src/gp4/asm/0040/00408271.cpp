#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Record of 0x78 bytes in the table at 0x0063bd7c; only the two velocity-like dwords are read.
struct Unknown_0063bd7c {
    uint8_t Unknown00[0x24];
    int32_t Unknown24;
    int32_t Unknown28;
    uint8_t Unknown2C[0x78 - 0x2c];
};

static __declspec(noinline) const Unknown_0063bd7c* recordAt(int32_t index) {
    return GP4_ARRAY(Unknown_0063bd7c, 0x0063bd7c) + index;
}

static inline int32_t magnitude(int32_t value) {
    if (value < 0) {
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
    }
    return value;
}

// ebx == -1: eax = (signed byte at [edx + esi + 0x218]) << 16. Otherwise eax is the larger
// magnitude of the two dwords at +0x24/+0x28 of record ebx in the 0x78-byte table at 0x0063bd7c,
// scaled by [0x006f2bec] >> 22. ebx and edx are preserved; the flags are those of the final
// shift (SHL by 16 or SHRD by 22).
// FUNCTION: GP4 0x00408271
void __cdecl FUN_00408271(gp4::Regs* r) {
    const int32_t index = static_cast<int32_t>(r->ebx);
    uint32_t result;
    uint32_t flags;

    if (index == -1) {
        const uint8_t* base = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
        const int32_t value = static_cast<int32_t>(static_cast<int8_t>(base[r->edx + 0x218u]));
        result = static_cast<uint32_t>(value) << 16;
        // pinned oracle model for SHL by n > 1: CF = last bit out, AF = 0,
        // OF = bit31(value << (n-1) ^ result)
        const uint32_t previous = static_cast<uint32_t>(value) << 15;
        flags = ((static_cast<uint32_t>(value) >> 16) & 1u)
            | parityFlag(result)
            | (result == 0u ? 0x40u : 0u)
            | ((result >> 24) & 0x80u)
            | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
    } else {
        const Unknown_0063bd7c* record = recordAt(index);
        int32_t largest = magnitude(record->Unknown24);
        const int32_t second = magnitude(record->Unknown28);
        if (largest < second) {
            largest = second;
        }
        const int32_t third = magnitude(record->Unknown28);
        if (largest < third) {
            largest = third;
        }
        const int64_t product = static_cast<int64_t>(largest) * GP4_GLOBAL(int32_t, 0x006f2bec);
        result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 22);
        // SHRD by 22: CF = bit 21 of the low dword, OF = bit31(shifted by 21 ^ result)
        const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 21);
        flags = (previous & 1u)
            | parityFlag(result)
            | (result == 0u ? 0x40u : 0u)
            | ((result >> 24) & 0x80u)
            | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408271, FUN_00408271, "regs(ebx:i32[-1..21], edx:u32[0..32], esi:ptr[0x260]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00408271")

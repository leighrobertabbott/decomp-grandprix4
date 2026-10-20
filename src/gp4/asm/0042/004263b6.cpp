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

struct Unknown_004263b6_Owner {
    uint8_t Unknown00[0x8d];
    uint8_t Unknown8D;
    uint8_t Unknown8E[0xba - 0x8e];
    uint16_t UnknownBA;
    uint8_t UnknownBC[0x240 - 0xbc];
    uint32_t Unknown240;
};

struct Unknown_004263b6_Source {
    uint8_t Unknown00[0x51];
    uint8_t Unknown51;
};

// Returns in eax the scaled word at owner+0xba (clamped to [0x00627b00]) when owner+0x240 is
// nonzero, or 0 when owner bit 1 of +0x8d is set or source bit 5 of +0x51 is set with bit 7 clear.
// edx is only changed by the scaling (high half of the product). The flags are those of the last
// "test" executed. ebp, ecx and edi are preserved.
// FUNCTION: GP4 0x004263b6
void __cdecl FUN_004263b6(gp4::Regs* r) {
    const Unknown_004263b6_Owner* owner = reinterpret_cast<const Unknown_004263b6_Owner*>(static_cast<uintptr_t>(r->esi));
    const Unknown_004263b6_Source* source = reinterpret_cast<const Unknown_004263b6_Source*>(static_cast<uintptr_t>(r->ebx));
    int32_t scaled = 0;

    if (owner->Unknown240 != 0u) {
        const int64_t product = static_cast<int64_t>(static_cast<uint32_t>(owner->UnknownBA)) * GP4_GLOBAL(int32_t, 0x00627afc);
        scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
        const int32_t limit = GP4_GLOBAL(int32_t, 0x00627b00);
        if (scaled > limit) {
            scaled = limit;
        }
        r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    }

    uint32_t flags;
    uint32_t result = 0u;
    const uint32_t ownerBits = owner->Unknown8D & 2u;
    if (ownerBits != 0u) {
        flags = testFlags8(ownerBits);
    } else {
        const uint8_t sourceByte = source->Unknown51;
        const uint32_t bit5 = sourceByte & 0x20u;
        flags = testFlags8(bit5);
        if (bit5 == 0u) {
            result = static_cast<uint32_t>(scaled);
        } else {
            const uint32_t bit7 = sourceByte & 0x80u;
            flags = testFlags8(bit7);
            if (bit7 != 0u) {
                result = static_cast<uint32_t>(scaled);
            }
        }
    }

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004263b6, FUN_004263b6, "regs(edx:u32, esi:ptr[0x248]:bytes, ebx:ptr[0x58]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

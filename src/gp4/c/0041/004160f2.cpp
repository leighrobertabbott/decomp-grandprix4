#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t low = value & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    return (low & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `cmp a, b` on 32-bit operands.
static inline uint32_t compareFlags32(uint32_t a, uint32_t b) {
    const uint32_t diff = a - b;
    uint32_t flags = parityFlag8(diff);
    if (a < b) flags |= 0x01u;
    if (((a ^ b ^ diff) & 0x10u) != 0u) flags |= 0x10u;
    if (diff == 0u) flags |= 0x40u;
    if ((diff & 0x80000000u) != 0u) flags |= 0x80u;
    if ((((a ^ b) & (a ^ diff)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

static inline uint32_t setLow16(uint32_t reg, uint32_t value) {
    return (reg & 0xffff0000u) | (value & 0xffffu);
}

// FUNCTION: GP4 0x004160f2
// Picks the 16-bit value stored to word +0xee of the car record in esi (ax on entry to the final
// store). The 0x0062fb48 marker is cleared first. With [0x00622424] set the value is 0 or 0x4000
// (bit 0 of +0x6f); otherwise it starts at the current word +0xee, is replaced by the result of
// 0x00415fe2 (eax/ecx/edx in, ax out) when the state bits ask for it, capped by [0x00638dbc]
// (negative: 0) while the +0x162 / +0x9c / +0x78 bits ask for it, forced to 0x4000 by the +0x8d /
// +0x84 combination, to 0 by +0x230 bit 3 and, while +0xe5 bit 2 and +0x8d bit 2 are set and word
// +0x9a >= [0x00603d58], capped at [0x00603d5c] (the sign-extended value, eax, is what is capped).
// Flags are those of the last test / cmp of the final +0xe5 / +0x8d / +0x9a block.
void __cdecl FUN_004160f2(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;

    GP4_GLOBAL(uint8_t, 0x0062fb48) = 0;

    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0u) {
        eax = setLow16(eax, (rec[0x6f] & 1u) != 0u ? 0x4000u : 0u);
    } else {
        eax = setLow16(eax, GP4_FIELD(uint16_t, rec, 0xee));

        bool pick;
        if ((rec[0xe5] & 4u) != 0u) {
            pick = (rec[0x179] & 1u) == 0u || (rec[0x203] & 0x20u) == 0u;
        } else {
            pick = (rec[0x78] & 0x10u) != 0u;
        }
        if (pick) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.eax = eax;
            call.ecx = ecx;
            call.edx = edx;
            gp4::call_regs(0x00415fe2, call);
            eax = call.eax;
            ecx = call.ecx;
            edx = call.edx;
        }

        const bool capped = (rec[0x162] & 8u) != 0u ||
                            ((rec[0xe5] & 4u) == 0u && ((rec[0x9c] & 1u) != 0u || (rec[0x78] & 1u) != 0u));
        if (capped) {
            int16_t limit = GP4_GLOBAL(int16_t, 0x00638dbc);
            if (limit < 0) {
                limit = 0;
            }
            edx = setLow16(edx, static_cast<uint16_t>(limit));
            if (static_cast<int16_t>(eax) > limit) {
                eax = setLow16(eax, static_cast<uint16_t>(limit));
            }
        }

        if ((rec[0x8d] & 0x10u) != 0u && (rec[0x162] & 8u) == 0u && rec[0x84] == 6u) {
            eax = setLow16(eax, 0x4000u);
        }
    }

    if ((rec[0xe5] & 4u) != 0u && (rec[0x230] & 8u) != 0u) {
        eax = setLow16(eax, 0u);
    }

    // test byte [esi+0xe5], 4 / test byte [esi+0x8d], 4 with a zero result: ZF and PF set.
    uint32_t flags = 0x44u;
    if ((rec[0xe5] & 4u) != 0u && (rec[0x8d] & 4u) != 0u) {
        const int32_t position = static_cast<int16_t>(GP4_FIELD(uint16_t, rec, 0x9a));
        const int32_t floorLimit = GP4_GLOBAL(int32_t, 0x00603d58);
        edx = static_cast<uint32_t>(position);
        flags = compareFlags32(edx, static_cast<uint32_t>(floorLimit));
        if (position >= floorLimit) {
            const int32_t value = static_cast<int16_t>(eax);
            eax = static_cast<uint32_t>(value);
            edx = GP4_GLOBAL(uint32_t, 0x00603d5c);
            flags = compareFlags32(eax, edx);
            if (value > static_cast<int32_t>(edx)) {
                eax = edx;
            }
        }
    }

    GP4_FIELD(uint16_t, rec, 0xee) = static_cast<uint16_t>(eax);

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004160f2, FUN_004160f2, "regs(esi:ptr[0x273]:bytes, edi:ptr[0x59]:bytes, eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

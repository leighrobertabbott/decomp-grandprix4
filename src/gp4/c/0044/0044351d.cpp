#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Scales a random byte (from the generator 0x00401462) by the table entry 0x00602ad0[word 0x007ad89e]
// and the constant 0x00602ae4 in Q14 fixed point: 0x4000 - ((byte << 6) * table >> 14), times the
// constant >> 14, stored as a word at +0x238 of the record in esi; bit 2 of byte +0xe5 sets bit 7 of
// byte +0x203. eax / edx keep the last product; the flags are those of the final test / or.
// FUNCTION: GP4 0x0044351d
void __cdecl FUN_0044351d(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs call = *r;
    gp4::call_regs(0x00401462, call);

    const int32_t scaled = static_cast<int32_t>((call.eax & 0xffu) << 6);
    const uint32_t index = GP4_GLOBAL(uint16_t, 0x007ad89e);
    const int32_t entry = GP4_ARRAY(int32_t, 0x00602ad0)[index];
    const int64_t first = static_cast<int64_t>(scaled) * entry;
    const uint32_t firstLow = static_cast<uint32_t>(first);
    const uint32_t firstHigh = static_cast<uint32_t>(first >> 32);
    const uint32_t shifted = (firstLow >> 14) | (firstHigh << 18);
    const int32_t complement = static_cast<int32_t>(0x4000u - shifted);

    const int32_t factor = GP4_GLOBAL(int32_t, 0x00602ae4);
    const int64_t second = static_cast<int64_t>(complement) * factor;
    const uint32_t secondLow = static_cast<uint32_t>(second);
    const uint32_t secondHigh = static_cast<uint32_t>(second >> 32);
    const uint32_t result = (secondLow >> 14) | (secondHigh << 18);

    *reinterpret_cast<uint16_t*>(record + 0x238) = static_cast<uint16_t>(result);

    uint32_t flags;
    const uint8_t mode = record[0xe5];
    if ((mode & 4u) != 0u) {
        const uint8_t updated = static_cast<uint8_t>(record[0x203] | 0x80u);
        record[0x203] = updated;
        flags = logicFlags8(updated);
    } else {
        flags = logicFlags8(mode & 4u);
    }

    r->eax = result;
    r->edx = secondHigh;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044351d, FUN_0044351d, "regs(esi:ptr[0x248]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_0042595c")

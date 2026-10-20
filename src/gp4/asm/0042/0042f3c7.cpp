#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a logic operation (test/or/and) with a 32-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit "neg value" (result = 0 - value).
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return parityFlag(result)
        | (value != 0u ? 1u : 0u)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// Steering-style output: word [esi+0x248] = +-max(0x4000 - |c|, 0xc00) where c is the sum of a
// scaled difference of the words at +0x90 of the record and of the block at [0x006043e4], and a
// random byte (generator 0x401462) scaled by [0x00603b90] plus [0x00603b94], negated when the word
// at [0x006269b8 + record[0x234] * 0x10] is negative. The sign of c is applied to the result.
// Leaves ecx = scaled difference, edx = high half of the random product, ebp = c, eax = result.
// FUNCTION: GP4 0x0042f3c7
void __cdecl FUN_0042f3c7(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(r->esi);
    uint8_t* other = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x006043e4));

    const int16_t selector = *reinterpret_cast<const int16_t*>(GP4_ARRAY(uint8_t, 0x006269b8) + record[0x234] * 0x10);

    const uint16_t difference16 = static_cast<uint16_t>(GP4_FIELD(uint16_t, other, 0x90) - GP4_FIELD(uint16_t, record, 0x90));
    const int32_t difference = static_cast<int16_t>(difference16);
    const int64_t product = static_cast<int64_t>(difference) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603b8c));
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    gp4::Regs c = *r;
    c.eax = scaled;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00401462, c);

    const int32_t random = static_cast<int32_t>((c.eax & 0xffu) << 6);
    const int64_t product2 = static_cast<int64_t>(random) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603b90));
    uint32_t noise = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 14) + GP4_GLOBAL(uint32_t, 0x00603b94);
    if (selector < 0) {
        noise = 0u - noise;
    }
    const uint32_t sum = noise + scaled;

    uint32_t magnitude = sum;
    if (static_cast<int32_t>(magnitude) < 0) {
        magnitude = 0u - magnitude;
    }
    uint32_t value = (0u - magnitude) + 0x4000u;
    if (static_cast<int32_t>(value) < 0xc00) {
        value = 0xc00u;
    }
    uint32_t flags;
    if (static_cast<int32_t>(sum) < 0) {
        flags = negFlags(value);
        value = 0u - value;
    } else {
        flags = logicFlags(sum);
    }
    GP4_FIELD(uint16_t, record, 0x248) = static_cast<uint16_t>(value);

    r->eax = value;
    r->ecx = scaled;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product2) >> 32);
    r->ebp = sum;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042f3c7, FUN_0042f3c7, "regs(esi:ptr[0x250]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_0042f3c7")

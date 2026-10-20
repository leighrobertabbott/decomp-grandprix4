#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `sub left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// imul word followed by two shl ax,1 / rcl dx,1 pairs: the high word of (a * b) << 2.
static inline uint16_t scaledProduct(uint16_t a, uint16_t b) {
    const int32_t product = static_cast<int32_t>(static_cast<int16_t>(a)) * static_cast<int32_t>(static_cast<int16_t>(b));
    return static_cast<uint16_t>(static_cast<uint32_t>(product) >> 14);
}

// Same blend as 0x00427695 (word pairs of the blocks at [esi+0x1b8] and edi weighted by the Q14
// word [esi+0x66], the second one through the slope term 0x00427579, mixed by flag bits of the
// esi record and scaled by the dword [esi+0x1a0]), but the result replaces the word [esi+0x142]
// and the change (new - old) goes to [esi+0x184]. Leaves ebx = [esi+0x1b8], ecx = (eax high :
// slope result), edx = (high word of [esi+0x1a0] : old [esi+0x142]) and eax = (high word left by
// the callee : change); the flags are those of the final 16-bit sub.
// FUNCTION: GP4 0x00427760
void __cdecl FUN_00427760(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* pair = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t blockAddress = GP4_FIELD(uint32_t, rec, 0x1b8);
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(blockAddress));
    const uint32_t eaxHigh = r->eax & 0xffff0000u;
    const uint16_t weight = GP4_FIELD(uint16_t, rec, 0x66);

    const uint16_t base1 = static_cast<uint16_t>(GP4_FIELD(uint16_t, block, 0x18) + GP4_FIELD(uint16_t, pair, 0));
    const uint16_t target1 = static_cast<uint16_t>(GP4_FIELD(uint16_t, block, 0x178) + GP4_FIELD(uint16_t, pair, 0x160));
    const uint16_t first = static_cast<uint16_t>(scaledProduct(static_cast<uint16_t>(target1 - base1), weight) + base1);

    const uint16_t base2 = GP4_FIELD(uint16_t, pair, 0xa);
    const uint16_t target2 = GP4_FIELD(uint16_t, pair, 0x16a);
    const uint16_t second = static_cast<uint16_t>(scaledProduct(static_cast<uint16_t>(target2 - base2), weight) + base2);

    gp4::Regs c = *r;
    c.eax = eaxHigh | second;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00427579, c);
    const uint16_t slope = static_cast<uint16_t>(c.eax);

    uint16_t blend = first;
    if ((GP4_FIELD(uint8_t, rec, 0xfe) & 0x40u) != 0u) {
        blend = static_cast<uint16_t>((static_cast<int16_t>(blend - slope) >> 1) + slope);
    }
    if ((GP4_FIELD(uint8_t, rec, 0x9c) & 0x10u) != 0u) {
        blend = slope;
    } else if ((GP4_FIELD(uint8_t, rec, 0x163) & 0x80u) != 0u) {
        if ((GP4_FIELD(uint8_t, rec, 0x122) & 0x80u) != 0u) {
            blend = static_cast<uint16_t>(static_cast<uint16_t>(0u - static_cast<uint16_t>(blend - slope)) + slope);
        } else if ((GP4_FIELD(uint8_t, rec, 0x163) & 0x40u) != 0u) {
            blend = slope;
        }
    }

    const uint32_t scale = GP4_FIELD(uint32_t, rec, 0x1a0);
    const uint16_t product = scaledProduct(static_cast<uint16_t>(blend - slope), static_cast<uint16_t>(scale));
    const uint16_t result = static_cast<uint16_t>(product + slope);
    const uint16_t previous = GP4_FIELD(uint16_t, rec, 0x142);
    GP4_FIELD(uint16_t, rec, 0x142) = result;
    const uint16_t change = static_cast<uint16_t>(result - previous);
    GP4_FIELD(uint16_t, rec, 0x184) = change;

    r->eax = (c.eax & 0xffff0000u) | change;
    r->ebx = blockAddress;
    r->ecx = eaxHigh | slope;
    r->edx = (scale & 0xffff0000u) | previous;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags16(result, previous);
}
GP4_IMPL(0x00427760, FUN_00427760, "regs(eax:u32, esi:ptr[1056]:layout=rec_004275d1, edi:ptr[0x1cc]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")

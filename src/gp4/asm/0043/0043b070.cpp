#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of "cmp left, right" (32-bit) without CF.
static inline uint32_t subFlagsNoCarry(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (a * b) >> 14 as "imul; shrd eax, edx, 14" leaves it in eax
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * b) >> 14);
}

// Q14 word table (16384 = 1.0) lookup of an angle: |angle| in 16 bits, byte offset (angle >> 2) & 0xfffe
static inline int32_t tableValue(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) angle = static_cast<uint16_t>(0u - angle);
    const uint32_t offset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return static_cast<int16_t>(*reinterpret_cast<const uint16_t*>(0x007c5c5cu + offset));
}

// FUNCTION: GP4 0x0043b070
// Tests whether a point is on the near side of a plane: with the angle in ax (16-bit, 0x4000 =
// a quarter turn) it takes sin = table[|ax|] (ecx) and cos = table[|0x4000 - ax|] (ebp), forms
// the signed distance A = +-((esi.x - 0x00623e30) * ecx - (esi.z - 0x00623e38) * ebp) >> 14 +
// [esi+0x68] (sign flipped when edx is negative) and returns CF = 1 when A >= DAT_00608284.
// Otherwise it adds the corresponding velocity term (esi+0x18/0x20, scaled by DAT_00603854 >> 16)
// and returns CF = (A + term + [esi+0x68] >= DAT_00608284). eax and ebx are restored; ecx,
// ebp and edx (the first term, or A on the second path) and the other flags are those of the last cmp.
void __cdecl FUN_0043b070(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const bool flip = static_cast<int32_t>(r->edx) < 0;
    const uint16_t angle = static_cast<uint16_t>(r->eax);

    const int32_t ecx = tableValue(angle);
    const int32_t ebp = tableValue(static_cast<uint16_t>(0x4000u - angle));
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00608284);
    const uint32_t bias = GP4_FIELD(uint32_t, rec, 0x68);

    const uint32_t zTerm = mulShr14(static_cast<int32_t>(GP4_FIELD(uint32_t, rec, 8) - GP4_GLOBAL(uint32_t, 0x00623e38)), ebp);
    const uint32_t xTerm = mulShr14(static_cast<int32_t>(GP4_FIELD(uint32_t, rec, 0) - GP4_GLOBAL(uint32_t, 0x00623e30)), ecx);
    uint32_t distance = xTerm - zTerm;
    if (flip) distance = 0u - distance;
    const uint32_t first = distance + bias;

    uint32_t flags = subFlagsNoCarry(first, static_cast<uint32_t>(limit));
    uint32_t edx = zTerm;
    bool carry = true;
    if (static_cast<int32_t>(first) < limit) {
        const uint32_t vzTerm = mulShr14(GP4_FIELD(int32_t, rec, 0x20), ebp);
        const uint32_t vxTerm = mulShr14(GP4_FIELD(int32_t, rec, 0x18), ecx);
        uint32_t velocity = vxTerm - vzTerm;
        if (flip) velocity = 0u - velocity;
        const uint32_t scaled = static_cast<uint32_t>(
            static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(velocity)) * GP4_GLOBAL(int32_t, 0x00603854)) >> 16);
        const uint32_t second = scaled + first + bias;
        flags = subFlagsNoCarry(second, static_cast<uint32_t>(limit));
        edx = first;
        carry = static_cast<int32_t>(second) >= limit;
    }

    r->ecx = static_cast<uint32_t>(ecx);
    r->ebp = static_cast<uint32_t>(ebp);
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags | (carry ? 1u : 0u);
}
GP4_IMPL(0x0043b070, FUN_0043b070, "regs(eax:u32, edx:u32, esi:ptr[0x70]:bytes) -> (ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

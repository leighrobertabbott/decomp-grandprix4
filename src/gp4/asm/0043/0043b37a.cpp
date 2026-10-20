#include <gp4/gp4.h>

// Keeps the compiler from folding record-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* OutBase() { return GP4_ARRAY(uint8_t, 0x00622455); }
static __declspec(noinline) uint8_t* SineTable() { return GP4_ARRAY(uint8_t, 0x007c5c5c); }

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// (value * scale) >> 16 as "imul; shrd eax, edx, 16": the low 32 bits of the shifted product.
static inline uint32_t mulShr16(int32_t value, int32_t scale) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(value) * static_cast<int64_t>(scale)) >> 16);
}

// Q14 table lookup at byte offset ((angle >> 2) & 0xfffe) for the 16-bit angle magnitude.
static inline int32_t tableLookup(uint16_t angle) {
    const uint32_t offset = (static_cast<uint32_t>(angle) >> 2) & 0xfffeu;
    return static_cast<int32_t>(GP4_FIELD(int16_t, SineTable(), offset));
}

// FUNCTION: GP4 0x0043b37a
// Fills the six dwords at 0x00622455 from the record at esi and the Q16 scale in eax; returns
// eax = 0x00622455 and the flags of the final add. All other registers are preserved.
void __cdecl FUN_0043b37a(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(r->esi);
    uint8_t* const out = OutBase();
    const int32_t scale = static_cast<int32_t>(r->eax);
    const bool negate = (GP4_FIELD(uint8_t, rec, 0x71) & 4u) != 0u;

    int32_t rem;
    const int32_t quotient = gp4::idiv64(
        static_cast<int64_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0xda)) << 7) * static_cast<int64_t>(scale),
        GP4_GLOBAL(int32_t, 0x00603854), &rem);

    int32_t lateral = static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x9a));
    if (negate) lateral = -lateral;
    lateral = static_cast<int32_t>(static_cast<uint32_t>(lateral) << 8) + quotient;
    GP4_FIELD(uint32_t, out, 0xc) = mulShr16(lateral, scale);

    uint32_t value = mulShr16(GP4_FIELD(int32_t, rec, 0xac), scale);
    if (static_cast<int32_t>(value) < 0) {
        value = 0u - value;
        if (value >= 0x400000u) value = 0x400000u;
        value = 0u - value;
    } else if (value >= 0x400000u) {
        value = 0x400000u;
    }
    GP4_FIELD(uint32_t, out, 0x10) = value << 8;
    GP4_FIELD(uint32_t, out, 0x14) = 0xbd00u;

    const int32_t angle = static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x18));
    GP4_FIELD(uint32_t, out, 8) = static_cast<uint32_t>(angle) << 16;

    uint16_t complement = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(angle));
    if (static_cast<int16_t>(complement) < 0) complement = static_cast<uint16_t>(0u - complement);
    const int32_t cosTerm = tableLookup(complement);

    uint16_t direct = static_cast<uint16_t>(angle);
    if (static_cast<int16_t>(direct) < 0) direct = static_cast<uint16_t>(0u - direct);
    const int32_t sinTerm = tableLookup(direct);

    const int32_t radius = negate ? -0x1de00 : 0x1de00;
    const uint32_t first = static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(radius) * cosTerm) >> 14);
    GP4_FIELD(uint32_t, out, 0) = first + GP4_FIELD(uint32_t, rec, 0);

    const uint32_t second = static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(radius) * sinTerm) >> 14);
    const uint32_t offsetY = GP4_FIELD(uint32_t, rec, 4);
    GP4_FIELD(uint32_t, out, 4) = second + offsetY;

    r->eax = 0x00622455u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(second, offsetY);
}
GP4_IMPL(0x0043b37a, FUN_0043b37a, "regs(eax:i32[-65536..65536], esi:ptr[0xf0]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0041a2ec")

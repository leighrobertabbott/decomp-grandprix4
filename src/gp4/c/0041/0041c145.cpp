#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of SHL r32, count (count > 1) in the pinned oracle model: CF = last bit out, AF = 0,
// OF = bit31(value << (count - 1) ^ result).
static inline uint32_t shlFlags32(uint32_t value, uint32_t count) {
    const uint32_t result = value << count;
    const uint32_t previous = value << (count - 1);
    return (previous >> 31)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The two 0x300-byte wheel-pair records at 0x006380d8 (front, rear): only the fields used here.
struct Unknown_006380d8 {
    uint8_t Unknown00[4];
    int32_t Unknown04;
    uint8_t Unknown08[0x48 - 0x08];
    int32_t Unknown48;
    uint8_t Unknown4C[0x1a0 - 0x4c];
    int32_t Unknown1A0;
    uint8_t Unknown1A4[0x258 - 0x1a4];
    int32_t Unknown258;
    int32_t Unknown25C;
    int32_t Unknown260;
    int32_t Unknown264;
    uint8_t Unknown268[0x300 - 0x268];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// edx:eax = (a - b) * gain; shrd eax, edx, 14. Returns the 64-bit product.
static inline int64_t wideProduct(int32_t difference, int32_t gain) {
    return static_cast<int64_t>(difference) * static_cast<int64_t>(gain);
}

// Per record, with s = ([0x623b84] >> 1) + rec.25C + rec.04:
//   rec.258 = ((s << 5) * [0x6013d4]) / [0x623af4] - rec.264
//   rec.260 = (s << 14) / [0x6013d4]
// Returns s (the value the final "shl eax, 14" shifts); *remainder is the last idiv's remainder.
static __forceinline uint32_t updateAxle(Unknown_006380d8* rec, int32_t* quotient, int32_t* remainder) {
    const uint32_t sum = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623b84) >> 1)
        + static_cast<uint32_t>(rec->Unknown25C) + static_cast<uint32_t>(rec->Unknown04);
    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(sum << 5))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006013d4));
    const int32_t position = gp4::idiv64(scaled, GP4_GLOBAL(int32_t, 0x00623af4), remainder);
    rec->Unknown258 = static_cast<int32_t>(static_cast<uint32_t>(position) - static_cast<uint32_t>(rec->Unknown264));
    *quotient = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(sum)) << 14,
                            GP4_GLOBAL(int32_t, 0x006013d4), remainder);
    rec->Unknown260 = *quotient;
    return sum;
}

// FUNCTION: GP4 0x0041c145
// Wheel-pair update of the records at 0x006380d8 (front = record 0, rear = record 1). The two axle
// terms rec.25C = (rec.48 - rec.1A0) * [0x6013d4] >> 14 feed the chain 0x41bc3a, 0x41bd56 (esi =
// pointer record), 0x41bbef, 0x41befb, 0x41c027, 0x41befb, 0x41c00c; then updateAxle runs for the
// front and the rear record. ecx and ebp are what the last 0x41befb left, eax/edx the rear
// division's quotient/remainder; the flags are those of the final "shl eax, 14" (idiv leaves them).
void __cdecl FUN_0041c145(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ebx = 0x006380d8u;
    Unknown_006380d8* records = reinterpret_cast<Unknown_006380d8*>(static_cast<uintptr_t>(c.ebx));

    const int64_t frontProduct = wideProduct(static_cast<int32_t>(
        static_cast<uint32_t>(records[0].Unknown48) - static_cast<uint32_t>(records[0].Unknown1A0)),
        GP4_GLOBAL(int32_t, 0x006013d4));
    records[0].Unknown25C = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(frontProduct) >> 14));

    const int64_t rearProduct = wideProduct(static_cast<int32_t>(
        static_cast<uint32_t>(records[1].Unknown48) - static_cast<uint32_t>(records[1].Unknown1A0)),
        GP4_GLOBAL(int32_t, 0x006013d4));
    const uint32_t rearTerm = static_cast<uint32_t>(static_cast<uint64_t>(rearProduct) >> 14);
    records[1].Unknown25C = static_cast<int32_t>(rearTerm);
    c.eax = rearTerm;
    c.edx = static_cast<uint32_t>(static_cast<uint64_t>(rearProduct) >> 32);

    gp4::call_regs(0x0041bc3a, c);
    gp4::call_regs(0x0041bd56, c);
    gp4::call_regs(0x0041bbef, c);
    gp4::call_regs(0x0041befb, c);
    gp4::call_regs(0x0041c027, c);
    gp4::call_regs(0x0041befb, c);
    gp4::call_regs(0x0041c00c, c);

    records = reinterpret_cast<Unknown_006380d8*>(static_cast<uintptr_t>(c.ebx));
    int32_t quotient;
    int32_t remainder;
    updateAxle(&records[0], &quotient, &remainder);
    const uint32_t rearSum = updateAxle(&records[1], &quotient, &remainder);

    r->eax = static_cast<uint32_t>(quotient);
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = static_cast<uint32_t>(remainder);
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | shlFlags32(rearSum, 14);
}
GP4_IMPL(0x0041c145, FUN_0041c145, "regs(esi:ptr[0x6a]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=dw_0041c027+state_0041bce9+state_0041b950+state_0041bd56+state_0041bbef+state_0041bd28+div_0041bc3a+state_0041befb+state_0041c145")

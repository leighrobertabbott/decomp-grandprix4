#include <gp4/gp4.h>

struct Unknown_00413170_Engine {
    uint8_t Unknown00[0x1d0];
    uint8_t Byte1D0;
    uint8_t Byte1D1;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// flags of "shrd reg32, reg32, 14" under the pinned oracle model:
// CF = last bit shifted out, AF = 0, OF = bit31(concat >> 13 xor concat >> 14).
static inline uint32_t shrd14Flags(uint32_t low, uint32_t high) {
    const uint64_t concat = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t previous = static_cast<uint32_t>(concat >> 13);
    const uint32_t result = static_cast<uint32_t>(concat >> 14);
    return ((low >> 13) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul (one operand, signed) followed by shrd eax, edx, 14: the product's bits 14..45. `high` receives
// edx (the high dword of the product) and `flags` the flags of the shrd.
static inline uint32_t mulShift14(uint32_t a, uint32_t b, uint32_t* high, uint32_t* flags) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    const uint32_t lo = static_cast<uint32_t>(static_cast<uint64_t>(product));
    const uint32_t hi = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    *high = hi;
    *flags = shrd14Flags(lo, hi);
    return (lo >> 14) | (hi << 18);
}

// FUNCTION: GP4 0x00413170
// edi = the track-segment record (also stored to the pair 0x00602d84/0x00602d88 the drivetrain reads),
// esi = the engine record. ebp = the low word of the 0x004058cf result, limited (signed) to [0x006224c5];
// eax = byte +0x1d0 << 6 goes through 0x00413139 (-> r). Then, with s1 = (byte +0x1d1 * [0x006224c1]) >> 14
// and s2 = (max(0x4000 - s1, 0) * [0x006224c5]) >> 14, the result is
// ((((s1 * ebp) >> 14) + s2) << 3) * r >> 14 * [0x00638d20] >> 14. ecx comes back as r, edx as the high
// dword of the last product and the flags are those of the last shrd.
void __cdecl FUN_00413170(gp4::Regs* r) {
    const Unknown_00413170_Engine* engine = reinterpret_cast<const Unknown_00413170_Engine*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint32_t, 0x00602d88) = r->edi;
    GP4_GLOBAL(uint32_t, 0x00602d84) = r->edi;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004058cf, call);
    uint32_t limited = call.eax & 0xffffu;

    call.eax = static_cast<uint32_t>(engine->Byte1D0) << 6;
    gp4::call_regs(0x00413139, call);
    const uint32_t scale = call.eax;

    const uint32_t cap = GP4_GLOBAL(uint32_t, 0x006224c5);
    if (static_cast<int32_t>(limited) > static_cast<int32_t>(cap)) {
        limited = cap;
    }

    uint32_t high;
    uint32_t flags;
    const uint32_t s1 = mulShift14(engine->Byte1D1, GP4_GLOBAL(uint32_t, 0x006224c1), &high, &flags);
    int32_t rest = static_cast<int32_t>(0x4000u - s1);
    if (rest < 0) {
        rest = 0;
    }
    const uint32_t s2 = mulShift14(static_cast<uint32_t>(rest), cap, &high, &flags);
    const uint32_t s3 = mulShift14(s1, limited, &high, &flags);
    uint32_t value = (s3 + s2) << 3;
    value = mulShift14(value, scale, &high, &flags);
    value = mulShift14(value, GP4_GLOBAL(uint32_t, 0x00638d20), &high, &flags);

    gp4::Regs out = call;
    out.eax = value;
    out.ecx = scale;
    out.edx = high;
    out.ebp = limited;
    out.eflags = (r->eflags & ~0x8d5u) | flags;
    *r = out;
}
GP4_IMPL(0x00413170, FUN_00413170, "regs(esi:ptr[0x242]:bytes, edi:rec[0x8943a0,0x160,0..29]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_00413090+engine_00413606")

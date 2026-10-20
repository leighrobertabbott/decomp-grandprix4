#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t add32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, 14: bits 14..45 of the signed 64-bit product.
static inline uint32_t mulShr14(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// edx after a one-operand imul: the high 32 bits of the signed product.
static inline uint32_t mulHigh(uint32_t a, uint32_t b) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

struct Unknown_00439f75 {
    uint8_t Unknown00[0x18];
    uint32_t Unknown18;
    uint8_t Unknown1C[4];
    uint32_t Unknown20;
    uint8_t Unknown24[0x48 - 0x24];
    uint32_t Unknown48;
    uint8_t Unknown4C[0x6e - 0x4c];
    uint8_t Unknown6E;
};

// One coordinate: both bodies move towards the weighted centre with the blend `weight` / `other`
// (Q14, weight + other = 0x4000) and the spring factor 0x006097f8. Returns the last product
// pieces the original leaves in eax, edx and the flags.
struct BlendOut { uint32_t eax; uint32_t edx; uint32_t flags; };

static inline BlendOut blendCoordinate(uint32_t* a, uint32_t* b, uint32_t weight, uint32_t other, uint32_t spring) {
    const uint32_t centre = mulShr14(*a, weight) + mulShr14(*b, other);
    const uint32_t delta = mulShr14(*b - *a, spring);
    *a = mulShr14(delta, other) + centre;
    const uint32_t pulled = 0u - mulShr14(delta, weight);
    *b = pulled + centre;
    BlendOut out;
    out.eax = pulled + centre;
    out.edx = mulHigh(delta, weight);
    out.flags = add32Flags(pulled, centre);
    return out;
}

// FUNCTION: GP4 0x00439f75
// Collision response between two bodies (ebx, edi): the mass fraction ratio = ([ebx+0x48] << 14) /
// ([ebx+0x48] + [edi+0x48]) (denominator 1 when zero) weights a blend of the coordinates at +0x18 and
// +0x20, which are moved towards each other by the spring factor 0x006097f8; the bytes +0x6e are
// cleared. eax, ecx, edx, ebp keep the last intermediate values (ebp = 0x4000 - ratio, ecx = ratio,
// edx = high word of the last product, eax = last stored value), esi is restored and the flags are
// those of the last "add eax, esi".
void __cdecl FUN_00439f75(gp4::Regs* r) {
    Unknown_00439f75* first = reinterpret_cast<Unknown_00439f75*>(static_cast<uintptr_t>(r->ebx));
    Unknown_00439f75* second = reinterpret_cast<Unknown_00439f75*>(static_cast<uintptr_t>(r->edi));

    const uint32_t massFirst = first->Unknown48;
    int32_t denominator = static_cast<int32_t>(massFirst + second->Unknown48);
    if (denominator == 0) denominator = 1;
    int32_t remainder;
    const int32_t ratio = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(massFirst << 14)), denominator, &remainder);
    const uint32_t weight = static_cast<uint32_t>(ratio);
    const uint32_t other = 0x4000u - weight;
    const uint32_t spring = GP4_GLOBAL(uint32_t, 0x006097f8);

    blendCoordinate(&first->Unknown18, &second->Unknown18, weight, other, spring);
    const BlendOut last = blendCoordinate(&first->Unknown20, &second->Unknown20, weight, other, spring);

    first->Unknown6E = 0;
    second->Unknown6E = 0;

    r->eax = last.eax;
    r->ecx = weight;
    r->edx = last.edx;
    r->ebp = other;
    r->eflags = (r->eflags & ~0x8d5u) | last.flags;
}
GP4_IMPL(0x00439f75, FUN_00439f75, "regs(ebx:ptr[0x76]:i32[-8..8], edi:ptr[0x76]:i32[-8..8]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

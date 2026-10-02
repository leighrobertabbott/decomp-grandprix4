#include <gp4/gp4.h>

struct Unknown_0042637c_Owner {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x25];
    uint8_t Unknown8D;
};

struct Unknown_0042637c_Source {
    uint8_t Unknown00[0xa4];
    uint16_t UnknownA4;
    uint16_t UnknownA6;
    uint8_t UnknownA8[0x15c];
    uint16_t Unknown204;
    uint16_t Unknown206;
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042637c
void __cdecl FUN_0042637c(gp4::Regs* r) {
    const Unknown_0042637c_Owner* owner = reinterpret_cast<const Unknown_0042637c_Owner*>(r->esi);
    const Unknown_0042637c_Source* source = reinterpret_cast<const Unknown_0042637c_Source*>(r->ebx);

    uint32_t base;
    uint32_t target;
    if ((owner->Unknown8D & 1u) != 0u) {
        base = source->UnknownA4;
        target = source->Unknown204;
    } else {
        base = source->UnknownA6;
        target = source->Unknown206;
    }
    const uint32_t delta = target - base;
    const int32_t scale = owner->Unknown66;

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(delta)) * static_cast<int64_t>(scale);
    const uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    r->eax = scaled;
    r->ecx = base + scaled;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->ebp = delta;
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(base, scaled);
}
GP4_IMPL(0x0042637c, FUN_0042637c, "regs(esi:ptr[0x95]:bytes, ebx:ptr[0x210]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>

struct Unknown_00438878 {
    uint8_t Unknown00[0xa4];
    uint32_t UnknownA4;
    uint32_t UnknownA8;
    uint8_t UnknownAC[0xa4];
    uint32_t Unknown150;
};

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00438878
void __cdecl FUN_00438878(gp4::Regs* r) {
    const Unknown_00438878* first = reinterpret_cast<const Unknown_00438878*>(r->esi);
    const Unknown_00438878* second = reinterpret_cast<const Unknown_00438878*>(r->ebx);
    GP4_GLOBAL(uint32_t, 0x00629dd4) = first->UnknownA4 - second->UnknownA4;
    GP4_GLOBAL(uint32_t, 0x00629dd8) = first->Unknown150 - second->Unknown150;
    const uint32_t left = first->UnknownA8;
    const uint32_t right = second->UnknownA8;
    const uint32_t result = left - right;
    GP4_GLOBAL(uint32_t, 0x00629ddc) = result;
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(left, right);
}
GP4_IMPL(0x00438878, FUN_00438878, "regs(ebx:ptr[0x158]:bytes, esi:ptr[0x158]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

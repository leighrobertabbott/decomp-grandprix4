#include <gp4/gp4.h>

struct Unknown_0041e156 {
    uint8_t Unknown00[0x4c];
    int32_t Unknown4C;
    int32_t Unknown50;
    int32_t Unknown54;
};

static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t sum) {
    uint32_t parity = sum & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (sum < a ? 0x001u : 0u)
        | ((parity & 1u) == 0u ? 0x004u : 0u)
        | (((a ^ b ^ sum) & 0x10u) ? 0x010u : 0u)
        | (sum == 0 ? 0x040u : 0u)
        | ((sum & 0x80000000u) ? 0x080u : 0u)
        | ((((a ^ sum) & (b ^ sum)) & 0x80000000u) ? 0x800u : 0u);
}

// Loads the three dwords at +0x4c of the body in ebx into DAT_00623e18.., runs the Q14
// matrix steps 0x00401adc and 0x00401893, then adds the pair of dwords at esi to
// DAT_00623e30 / DAT_00623e38.
// FUNCTION: GP4 0x0041e156
void __cdecl FUN_0041e156(gp4::Regs* r) {
    const Unknown_0041e156* body = reinterpret_cast<const Unknown_0041e156*>(r->ebx);
    const int32_t* offset = reinterpret_cast<const int32_t*>(r->esi);

    GP4_GLOBAL(int32_t, 0x00623e18) = body->Unknown4C;
    GP4_GLOBAL(int32_t, 0x00623e1c) = body->Unknown50;
    GP4_GLOBAL(int32_t, 0x00623e20) = body->Unknown54;
    gp4::call_regs(0x00401adc, *r);
    gp4::call_regs(0x00401893, *r);

    const uint32_t first = GP4_GLOBAL(uint32_t, 0x00623e30) + (uint32_t)offset[0];
    GP4_GLOBAL(uint32_t, 0x00623e30) = first;

    const uint32_t lhs = GP4_GLOBAL(uint32_t, 0x00623e38);
    const uint32_t rhs = (uint32_t)offset[1];
    const uint32_t second = lhs + rhs;
    GP4_GLOBAL(uint32_t, 0x00623e38) = second;

    r->eax = second;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(lhs, rhs, second);
}
GP4_IMPL(0x0041e156, FUN_0041e156, "regs(ebx:ptr[0x5c]:bytes, esi:ptr[8]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

// 64-bit signed product of two dwords (imul with edx:eax), shifted right by 8 (shrd eax, edx, 8).
static inline uint32_t scaledProduct(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 8);
}

// Scales the vector 0x006081ec/f0/f4 by 0x00607fa0 (24.8 fixed point) into 0x00622b40/44/48; when
// any component is non-zero the rotation of that vector (angles negated y, x, z at slot 0x40) is
// composed with the one built from the angles at 0x00607f4c/50/54 (slot 0x20), the product (slot
// 0x60) is turned back into angles and they replace 0x00607f4c/50/54. Byte 0x006097a8 is cleared
// at the end. With an all-zero vector eax = 0 and edx = the high dword of the last product, the
// flags are those of `or eax, [0x00622b48]` (zero). Otherwise the registers and flags are those the
// last callee leaves, with eax = the last angle.
// FUNCTION: GP4 0x00402bb9
void __cdecl FUN_00402bb9(gp4::Regs* r) {
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00607fa0);
    uint32_t high = 0u;
    GP4_GLOBAL(uint32_t, 0x00622b40) = scaledProduct(GP4_GLOBAL(int32_t, 0x006081ec), scale, &high);
    GP4_GLOBAL(uint32_t, 0x00622b44) = scaledProduct(GP4_GLOBAL(int32_t, 0x006081f0), scale, &high);
    GP4_GLOBAL(uint32_t, 0x00622b48) = scaledProduct(GP4_GLOBAL(int32_t, 0x006081f4), scale, &high);

    const uint32_t x = GP4_GLOBAL(uint32_t, 0x00622b40);
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x00622b44);
    const uint32_t z = GP4_GLOBAL(uint32_t, 0x00622b48);

    if ((x | y | z) == 0u) {
        GP4_GLOBAL(uint8_t, 0x006097a8) = 0;
        r->eax = 0u;
        r->edx = high;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0u - y;
    c.ebx = x;
    c.ecx = z;
    c.edx = 0x40u;
    gp4::call_regs(0x00402655, c);

    c.eax = GP4_GLOBAL(uint32_t, 0x00607f50);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00607f4c);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00607f54);
    c.edx = 0x20u;
    gp4::call_regs(0x00402655, c);

    gp4::call_regs(0x00402564, c);
    c.edx = 0x60u;
    gp4::call_regs(0x00402777, c);
    gp4::call_regs(0x00402a9a, c);

    GP4_GLOBAL(uint32_t, 0x00607f4c) = GP4_GLOBAL(uint32_t, 0x00622bd0);
    GP4_GLOBAL(uint32_t, 0x00607f50) = GP4_GLOBAL(uint32_t, 0x00622bd4);
    c.eax = GP4_GLOBAL(uint32_t, 0x00622bd8);
    GP4_GLOBAL(uint32_t, 0x00607f54) = c.eax;
    GP4_GLOBAL(uint8_t, 0x006097a8) = 0;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00402bb9, FUN_00402bb9, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

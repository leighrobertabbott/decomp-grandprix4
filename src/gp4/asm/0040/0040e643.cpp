#include <gp4/gp4.h>

struct Unknown_0040e643 {
    uint8_t Unknown00[0x0e];
    uint16_t Angle0e;
    uint16_t Unknown10;
    uint16_t Angle12;
    uint16_t Unknown14;
    uint16_t Angle16;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of `shl value, 16` (oracle model: AF = 0, OF = bit31 of ((value << 15) ^ result)).
static inline uint32_t shl16Flags(uint32_t value) {
    const uint32_t result = value << 16;
    return ((value >> 16) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((value << 15) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040e643
// Takes the three binary angles at esi+0x0e / +0x12 / +0x16 and the angle triple at 0x00638dd2 /
// 0x00638dd6 / 0x00638dda, turns each into a quaternion (0x0040e0e5), stores the first in
// 0x00625a8c.. and multiplies the conjugate of the second by it (0x0040dff6), rotates the vector
// part through the matrix at 0x00623e3c (0x004017b1) and converts the quaternion
// (product w, rotated vector) back to three angles (0x0040e230), stored shifted left by 16 at
// 0x00638030 / 0x00638034 / 0x00638038. ebp is left as 0x004017b1 leaves it; the flags are those of
// the final `shl ecx, 16`.
void __cdecl FUN_0040e643(gp4::Regs* r) {
    const Unknown_0040e643* rec = reinterpret_cast<const Unknown_0040e643*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs rg = *r;
    rg.st_in = 0;
    rg.st_out = 0;
    rg.st_xin_mask = 0;
    rg.st_xout_mask = 0;

    rg.eax = (rg.eax & 0xffff0000u) | rec->Angle0e;
    rg.ebx = (rg.ebx & 0xffff0000u) | rec->Angle12;
    rg.ecx = (rg.ecx & 0xffff0000u) | rec->Angle16;
    gp4::call_regs(0x0040e0e5, rg);
    GP4_GLOBAL(uint32_t, 0x00625a8c) = rg.eax;
    GP4_GLOBAL(uint32_t, 0x00625a90) = rg.ebx;
    GP4_GLOBAL(uint32_t, 0x00625a94) = rg.ecx;
    GP4_GLOBAL(uint32_t, 0x00625a98) = rg.edx;

    rg.eax = (rg.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638dd2);
    rg.ebx = (rg.ebx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638dd6);
    rg.ecx = (rg.ecx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638dda);
    gp4::call_regs(0x0040e0e5, rg);
    rg.ebx = 0u - rg.ebx;
    rg.ecx = 0u - rg.ecx;
    rg.edx = 0u - rg.edx;
    gp4::call_regs(0x0040dff6, rg);

    const uint32_t product = rg.eax;
    GP4_GLOBAL(uint32_t, 0x00623e30) = rg.ebx;
    GP4_GLOBAL(uint32_t, 0x00623e34) = rg.ecx;
    GP4_GLOBAL(uint32_t, 0x00623e38) = rg.edx;
    gp4::call_regs(0x004017b1, rg);

    rg.ebx = GP4_GLOBAL(uint32_t, 0x00623e24);
    rg.ecx = GP4_GLOBAL(uint32_t, 0x00623e28);
    rg.edx = GP4_GLOBAL(uint32_t, 0x00623e2c);
    rg.eax = product;
    gp4::call_regs(0x0040e230, rg);

    const uint32_t flags = shl16Flags(rg.ecx);
    rg.eax <<= 16;
    rg.ebx <<= 16;
    rg.ecx <<= 16;
    GP4_GLOBAL(uint32_t, 0x00638030) = rg.eax;
    GP4_GLOBAL(uint32_t, 0x00638034) = rg.ebx;
    GP4_GLOBAL(uint32_t, 0x00638038) = rg.ecx;

    r->eax = rg.eax;
    r->ebx = rg.ebx;
    r->ecx = rg.ecx;
    r->edx = rg.edx;
    r->ebp = rg.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e643, FUN_0040e643, "regs(esi:ptr[0x20]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rot_00623e3c")

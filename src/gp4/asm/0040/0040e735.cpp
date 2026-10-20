#include <gp4/gp4.h>

struct Unknown_0040e735_Body {
    uint8_t Unknown00[0x9c];
    int32_t Unknown9C;
    int32_t UnknownA0;
    int32_t UnknownA4;
    uint8_t UnknownA8[0xc2 - 0xa8];
    uint16_t UnknownC2;
    uint8_t UnknownC4[0xc6 - 0xc4];
    uint16_t UnknownC6;
    uint8_t UnknownC8[0xca - 0xc8];
    uint16_t UnknownCA;
};

// (value * DAT_00603854) >> 16 as `imul; shrd eax, edx, 16` leaves it: the low 32 bits of the signed
// 64-bit product shifted right by 16. The high half of the product is returned through `high`.
static inline uint32_t scaleQ16(int32_t value, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);
}

// FUNCTION: GP4 0x0040e735
// esi = body. The vector (+0x9c, +0xa0, +0xa4) scaled by DAT_00603854 (>> 16) is stored at
// 0x00623e30/34/38 and normalised by 0x0040e46e into the quaternion (0x00625a8c..98); the three
// angle words (+0xc6, +0xc2, +0xca) are turned into an Euler quaternion by 0x0040e0e5, multiplied
// with it by 0x0040dff6 and converted back to angles by 0x0040e230, which are stored to the same
// words. Registers and flags come back as the last callee leaves them.
void __cdecl FUN_0040e735(gp4::Regs* r) {
    Unknown_0040e735_Body* body = reinterpret_cast<Unknown_0040e735_Body*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint32_t high;
    GP4_GLOBAL(uint32_t, 0x00623e30) = scaleQ16(body->Unknown9C, &high);
    GP4_GLOBAL(uint32_t, 0x00623e34) = scaleQ16(body->UnknownA0, &high);
    GP4_GLOBAL(uint32_t, 0x00623e38) = scaleQ16(body->UnknownA4, &high);

    s.eax = GP4_GLOBAL(uint32_t, 0x00623e30);
    s.ebx = GP4_GLOBAL(uint32_t, 0x00623e34);
    s.ecx = GP4_GLOBAL(uint32_t, 0x00623e38);
    s.edx = high;
    gp4::call_regs(0x0040e46e, s);

    GP4_GLOBAL(uint32_t, 0x00625a8c) = s.eax;
    GP4_GLOBAL(uint32_t, 0x00625a90) = s.ebx;
    GP4_GLOBAL(uint32_t, 0x00625a94) = s.ecx;
    GP4_GLOBAL(uint32_t, 0x00625a98) = s.edx;

    s.eax = (s.eax & 0xffff0000u) | body->UnknownC6;
    s.ebx = (s.ebx & 0xffff0000u) | body->UnknownC2;
    s.ecx = (s.ecx & 0xffff0000u) | body->UnknownCA;
    gp4::call_regs(0x0040e0e5, s);
    gp4::call_regs(0x0040dff6, s);
    gp4::call_regs(0x0040e230, s);

    body->UnknownC6 = static_cast<uint16_t>(s.eax);
    body->UnknownC2 = static_cast<uint16_t>(s.ebx);
    body->UnknownCA = static_cast<uint16_t>(s.ecx);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x0040e735, FUN_0040e735, "regs(esi:ptr[0xd2]:i32[-0x4000..0x4000]) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

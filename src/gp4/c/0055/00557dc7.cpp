#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00557dc7
// Distance between the first track-segment record (0x008943a0) and the previous one of the lap end
// [0x00866ecc] (dwords +0xf0 and +0xf8): sqrt(dx*dx + dz*dz), computed on the x87 stack (fild,
// fmul st(0), faddp, fsqrt) and stored with fistp to [0x006f2bf4]. The record before the lap end gets
// the distance >> 4 as a word at +0x130. Registers: ebx = 0x8943a0, edi = that record, eax = distance
// >> 4, edx = dz; the flags are those of `sar eax, 4`.
void __cdecl FUN_00557dc7(gp4::Regs* r) {
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x008943a0));
    uint8_t* prev = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x160u));

    const int32_t dx = static_cast<int32_t>(GP4_FIELD(uint32_t, first, 0xf0) - GP4_FIELD(uint32_t, prev, 0xf0));
    const int32_t dz = static_cast<int32_t>(GP4_FIELD(uint32_t, first, 0xf8) - GP4_FIELD(uint32_t, prev, 0xf8));
    const gp4::x87::Ext sx = gp4::x87::ext_i32(dx);
    const gp4::x87::Ext sz = gp4::x87::ext_i32(dz);
    const gp4::x87::Ext sum = gp4::x87::operator*(sx, sx) + gp4::x87::operator*(sz, sz);
    const int32_t distance = gp4::x87::fistp32(gp4::x87::fsqrt(sum));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = distance;

    const int32_t shifted = distance >> 4;
    GP4_FIELD(uint16_t, prev, 0x130) = static_cast<uint16_t>(shifted);

    r->eax = static_cast<uint32_t>(shifted);
    r->ebx = 0x008943a0u;
    r->edx = static_cast<uint32_t>(dz);
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(prev));
    const uint32_t flags = gp4::flags::szp(static_cast<uint32_t>(shifted), 32)
                         | ((static_cast<uint32_t>(distance) >> 3) & 1u);       // CF = last bit shifted out
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00557dc7, FUN_00557dc7, "regs() -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00406ac7")

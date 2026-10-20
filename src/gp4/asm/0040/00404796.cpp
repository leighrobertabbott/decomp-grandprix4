#include <gp4/gp4.h>

// FUNCTION: GP4 0x00404796
// Wrapper around 0x004046d1 (rotate the 2D vector in 0x934df0/0x934df4 by the angle at [edi] and
// add the record's words +4/+8 scaled by 0x800). It saves the four working globals
// 0x934de8..0x934df4, seeds the vector with
//   x = eax << 8
//   y = ((0x40000 - u, floored at 0x100, signed) * edx >> 14) + (u >> 1 arithmetic)
// where u = (x * word[edi+0x14]) >> 14, runs the rotation, returns the rotated vector in
// eax (x) / edx (y), restores the four globals, and leaves the flags of the rotation's last add.
// ebx, ecx and ebp are preserved.
static inline uint32_t Shr14(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

void __cdecl FUN_00404796(gp4::Regs* r) {
    const uint32_t angleIn = r->eax;
    const int32_t scale = static_cast<int32_t>(r->edx);
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const uint32_t savedCos = GP4_GLOBAL(uint32_t, 0x00934de8);
    const uint32_t savedSin = GP4_GLOBAL(uint32_t, 0x00934dec);
    const uint32_t savedX = GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t savedY = GP4_GLOBAL(uint32_t, 0x00934df4);

    const int32_t x = static_cast<int32_t>(angleIn << 8);
    GP4_GLOBAL(uint32_t, 0x00934df0) = static_cast<uint32_t>(x);

    const int32_t k = GP4_FIELD(int16_t, rec, 0x14);
    const uint32_t u = Shr14(static_cast<int64_t>(x) * k);
    const int32_t half = static_cast<int32_t>(u) >> 1;
    int32_t gain = static_cast<int32_t>(0x40000u - u);
    if (gain < 0x100) gain = 0x100;
    GP4_GLOBAL(uint32_t, 0x00934df4) = Shr14(static_cast<int64_t>(gain) * scale) + static_cast<uint32_t>(half);

    gp4::Regs call = *r;
    gp4::call_regs(0x004046d1, call);

    const uint32_t outX = GP4_GLOBAL(uint32_t, 0x00934df0);
    const uint32_t outY = GP4_GLOBAL(uint32_t, 0x00934df4);

    GP4_GLOBAL(uint32_t, 0x00934df4) = savedY;
    GP4_GLOBAL(uint32_t, 0x00934df0) = savedX;
    GP4_GLOBAL(uint32_t, 0x00934dec) = savedSin;
    GP4_GLOBAL(uint32_t, 0x00934de8) = savedCos;

    r->eax = outX;
    r->edx = outY;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00404796, FUN_00404796, "regs(eax:u32, edi:ptr[0x40]:bytes, edx:i32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// Lateral-position term of a 0x160-byte track record (edi): the word at +0x60 plus
// 0x50, shifted left by 8, scales (fild +0x150 * f64 [0x6f2a04]); the product is
// rounded with fistp into [0x6f2bf4] and the dword at +0xf4 is added.
// Leaves eax = (word + 0x50) << 8 and ecx = fistp + [edi+0xf4] with the flags of the add.

// FUNCTION: GP4 0x005539f0
void __cdecl FUN_005539f0(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->edi));
    const int32_t slope = GP4_FIELD(int32_t, record, 0x150);
    const uint32_t width = GP4_FIELD(uint16_t, record, 0x60);
    const uint32_t height = GP4_FIELD(uint32_t, record, 0xf4);

    // eax = (word + 0x50) << 8, stored then reloaded with FILD
    const uint32_t factor = (width + 0x50u) << 8;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = static_cast<int32_t>(factor);

    // fild * fmul qword * fild * fmulp stay on the x87 stack (80-bit) until the fistp
    const gp4::x87::Ext product =
        (gp4::x87::ext_i32(slope) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2a04))) *
        gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const int32_t scaled = gp4::x87::fistp32(product);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;

    const uint32_t flags = gp4::flags::add32(static_cast<uint32_t>(scaled), height);

    r->eax = factor;
    r->ecx = static_cast<uint32_t>(scaled) + height;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005539f0, FUN_005539f0, "regs(edi:ptr[0x158]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=layout=multiply_0040ce7b")

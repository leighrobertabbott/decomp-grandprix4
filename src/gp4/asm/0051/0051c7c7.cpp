#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stretches 32 rows (stride 0x100) of a byte image: each row's `count` (ecx) bytes at base+eax are
// copied forward, byte by byte, to a position `delta` (edx, signed) away, then |delta| further bytes
// are filled with the last byte written (the byte before the fill position). Also saves the count
// in [0x00817430]. pushal/popal keep every register; the flags are those of the last `dec ebp`
// (CF from the final `add edi, 0x100`, result zero).
// FUNCTION: GP4 0x0051c7c7
void __cdecl FUN_0051c7c7(gp4::Regs* r) {
    const uint32_t count = r->ecx;
    const int32_t delta = static_cast<int32_t>(r->edx);
    uint8_t* src = reinterpret_cast<uint8_t*>(r->edi + r->eax);
    uint8_t* dst = src + delta;
    GP4_GLOBAL(uint32_t, 0x00817430) = count;

    uint32_t fill = static_cast<uint32_t>(delta);
    if (delta < 0) {
        fill = 0u - fill;
    }

    for (int row = 0; row < 0x20; ++row) {
        for (uint32_t i = 0; i < count; ++i) {
            dst[i] = src[i];
        }
        if (fill != 0u) {
            const uint8_t last = dst[static_cast<int32_t>(count) - 1];
            for (uint32_t i = 0; i < fill; ++i) {
                dst[count + i] = last;
            }
        }
        if (row != 0x1f) {
            src += 0x100;
            dst += 0x100;
        }
    }

    const uint32_t f = gp4::flags::dec32(1u, gp4::flags::add32(reinterpret_cast<uint32_t>(dst), 0x100u));
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c7c7, FUN_0051c7c7, "regs(eax:u32[0x40..0x60], ecx:u32[0..64], edx:i32[-32..32], edi:ptr[0x2200]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

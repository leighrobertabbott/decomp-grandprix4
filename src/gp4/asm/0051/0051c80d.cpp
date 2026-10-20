#include <gp4/gp4.h>
#include <gp4/flags.h>

// Backward-copying twin of 0x0051c7c7: for each of 32 rows (stride 0x100) of a byte image, the
// `count` (ecx, at least 1) bytes at base+eax are copied from the high end down to base+eax+delta
// (edx, signed), then |delta| further bytes BELOW that destination are filled with the last byte
// read (the row's first source byte, as seen after the copy overwrote any overlap). Also saves the
// count in [0x00817430]. pushal/popal keep every register; the flags are those of the last
// `dec ebp` (CF from the final row's `add edi, 0x100`, result zero).
// FUNCTION: GP4 0x0051c80d
void __cdecl FUN_0051c80d(gp4::Regs* r) {
    const uint32_t count = r->ecx;
    const int32_t delta = static_cast<int32_t>(r->edx);
    const uint32_t base = r->edi + r->eax;
    GP4_GLOBAL(uint32_t, 0x00817430) = count;

    uint32_t srcEnd = base + count;
    uint32_t dstEnd = base + static_cast<uint32_t>(delta) + count;
    const uint32_t fillCount = delta < 0 ? 0u - static_cast<uint32_t>(delta) : static_cast<uint32_t>(delta);
    uint32_t lastDst = dstEnd;

    for (int row = 0; row < 0x20; ++row) {
        lastDst = dstEnd;
        uint32_t s = srcEnd;
        uint32_t d = dstEnd;
        uint32_t n = count;
        uint8_t last = 0u;
        do {
            --s;
            --d;
            last = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(s));
            *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(d)) = last;
            --n;
        } while (n != 0u);
        uint32_t f = fillCount;
        if (f != 0u) {
            do {
                --d;
                *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(d)) = last;
                --f;
            } while (f != 0u);
        }
        srcEnd += 0x100u;
        dstEnd += 0x100u;
    }

    const uint32_t flags = gp4::flags::dec32(1u, gp4::flags::add32(lastDst, 0x100u));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051c80d, FUN_0051c80d, "regs(eax:u32[0x40..0x60], ecx:u32[1..64], edx:i32[-32..32], edi:ptr[0x2200]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

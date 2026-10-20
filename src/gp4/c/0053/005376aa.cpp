#include <gp4/gp4.h>
#include <gp4/flags.h>

// Ring-index initialiser run by the 0x00537508 start-up chain, a pushal/popal wrapper (every register comes
// back). When bit 7 of byte +0xd of the block at 0x008299a8 is set it lays out the slot area:
// base [0x7a8e18] = dword +0x14 of the block, [0x7a8e30] = [0x7a8e20] * [0x7a8e24] + base,
// [0x7a8e28] = [0x7a8e30] + 8, [0x7a8e2c] = [0x7a8e1c] * 0x5b8 + [0x7a8e28], and fills two 4-dword index
// rings at 0x679b9c (0, 1, 3, 3 + N) and 0x679bac (0, 2, 2 + N, 3 + N) with N = [0x7a8e1c]. popal does not
// restore the flags: they are those of the last flag writer, `add eax, 1` (3 + N) on the taken path or the
// `test byte [edx + 0xd], 0x80` on the other.
// FUNCTION: GP4 0x005376aa
void __cdecl FUN_005376aa(gp4::Regs* r) {
    uint8_t* block = GP4_ARRAY(uint8_t, 0x008299a8);
    const uint32_t sel = GP4_FIELD(uint8_t, block, 0x0d) & 0x80u;
    uint32_t flags = gp4::flags::logic8(sel);
    if (sel != 0u) {
        const uint32_t base = GP4_FIELD(uint32_t, block, 0x14);
        GP4_GLOBAL(uint32_t, 0x007a8e18) = base;
        const uint32_t stride = GP4_GLOBAL(uint32_t, 0x007a8e24);
        const uint32_t count = GP4_GLOBAL(uint32_t, 0x007a8e20);
        const uint32_t area = count * stride + base;
        GP4_GLOBAL(uint32_t, 0x007a8e30) = area;
        GP4_GLOBAL(uint32_t, 0x007a8e28) = area + 8u;
        GP4_GLOBAL(uint32_t, 0x007a8e2c) = GP4_GLOBAL(uint32_t, 0x007a8e1c) * 0x5b8u + GP4_GLOBAL(uint32_t, 0x007a8e28);

        uint32_t* ring_a = GP4_ARRAY(uint32_t, 0x00679b9c);
        ring_a[0] = 0u;
        ring_a[1] = 1u;
        ring_a[2] = 3u;
        ring_a[3] = 3u + GP4_GLOBAL(uint32_t, 0x007a8e1c);

        uint32_t* ring_b = GP4_ARRAY(uint32_t, 0x00679bac);
        ring_b[0] = 0u;
        ring_b[1] = 2u;
        const uint32_t last = 2u + GP4_GLOBAL(uint32_t, 0x007a8e1c);
        ring_b[2] = last;
        ring_b[3] = last + 1u;
        flags = gp4::flags::add32(last, 1u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005376aa, FUN_005376aa, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=desc_005376aa")

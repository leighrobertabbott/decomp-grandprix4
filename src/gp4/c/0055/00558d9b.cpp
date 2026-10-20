#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Rebuilds the packed segment-attribute staging area. For every 0x160-byte segment record from
// [0x866ec0] up to (excluding) [0x866eb8], copies 0x18 dwords (0x60 bytes) to the running
// destination starting at 0x6f6dc4: the entry at 0x9559f8, or the next one at 0x955a58 when bit 2
// of the record's byte +0x48 is set. The end of the destination goes to [0x9559e8] (an `int3`
// assertion fires when it reaches 0x738fe4) and its length from 0x6f6dc4 to [0x9559ec].
// pushal/popal wrapper: every GPR is restored; the flags are those of the closing
// `sub edi, 0x6f6dc4`.
// FUNCTION: GP4 0x00558d9b
void __cdecl FUN_00558d9b(gp4::Regs* r) {
    uint32_t dest = 0x006f6dc4u;
    uint32_t rec = GP4_GLOBAL(uint32_t, 0x00866ec0);
    do {
        uint32_t source = 0x009559f8u;
        if ((GP4_FIELD(uint8_t, rec, 0x48) & 4u) != 0u) {
            source += 0x60u;
        }
        for (uint32_t i = 0u; i < 0x18u; ++i) {
            GP4_FIELD(uint32_t, dest, i * 4u) = GP4_FIELD(uint32_t, source, i * 4u);
        }
        dest += 0x60u;
        rec += 0x160u;
    } while (rec != GP4_GLOBAL(uint32_t, 0x00866eb8));
    GP4_GLOBAL(uint32_t, 0x009559e8) = dest;
    if (dest >= 0x00738fe4u) {
        __debugbreak();
    }
    GP4_GLOBAL(uint32_t, 0x009559ec) = dest - 0x006f6dc4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(dest, 0x006f6dc4u));
}
GP4_IMPL(0x00558d9b, FUN_00558d9b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+ring_0043d30c+segs_00558d9b+src_00558d9b")

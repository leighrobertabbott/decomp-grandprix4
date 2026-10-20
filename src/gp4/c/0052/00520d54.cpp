#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte length of [start, end): kept out of line so MSVC does not fold the two addresses into one
// constant (the original loads both as globals).
static __declspec(noinline) uint32_t Span(uint32_t end, uint32_t start) {
    return end - start;
}

// FUNCTION: GP4 0x00520d54
// pushal wrapper that serialises the game state into the buffer [0x006975c8] (skipped when it is
// null; the flags are then those of the null test). It copies the 0x7ad892 word to 0x66d8ba, runs
// 0x513982 (pointers -> offsets), then writes in order: [0x6975c0] bytes from 0x66d810, the 0xa5e
// dwords of 0x40b511, [0x7a8e20] blocks of [0x6975c4] bytes read contiguously from [0x7a8e18], 0x20
// bytes from 0x679b24 and 0x2c bytes from 0x7a9124, then runs 0x5139ed (offsets -> pointers) and
// 0x520cf4. popal restores every register; the flags are those of the last callee (0x520cf4).
void __cdecl FUN_00520d54(gp4::Regs* r) {
    const uint32_t buffer = GP4_GLOBAL(uint32_t, 0x006975c8);
    uint32_t f = gp4::flags::logic32(buffer);
    if (buffer != 0u) {
        gp4::Regs c = *r;
        GP4_GLOBAL(uint16_t, 0x0066d8ba) = GP4_GLOBAL(uint16_t, 0x007ad892);
        gp4::call_regs(0x00513982, c);

        // rep movsd: [0x6975c0] / 4 dwords from 0x66d810 into the buffer.
        c.edi = buffer;
        c.esi = 0x0066d810u;
        c.ecx = GP4_GLOBAL(uint32_t, 0x006975c0) >> 2;
        while (c.ecx != 0u) {
            GP4_GLOBAL(uint32_t, c.edi) = GP4_GLOBAL(uint32_t, c.esi);
            c.esi += 4u;
            c.edi += 4u;
            c.ecx -= 1u;
        }
        gp4::call_regs(0x0040b511, c);

        c.edi = c.eax;
        c.edx = 0u;
        c.esi = GP4_GLOBAL(uint32_t, 0x007a8e18);
        do {
            c.ecx = GP4_GLOBAL(uint32_t, 0x006975c4) >> 2;
            while (c.ecx != 0u) {
                GP4_GLOBAL(uint32_t, c.edi) = GP4_GLOBAL(uint32_t, c.esi);
                c.esi += 4u;
                c.edi += 4u;
                c.ecx -= 1u;
            }
            c.edx += 1u;
        } while (c.edx != GP4_GLOBAL(uint32_t, 0x007a8e20));

        c.esi = 0x00679b24u;
        c.ecx = Span(0x00679b44u, c.esi);
        while (c.ecx != 0u) {
            GP4_GLOBAL(uint8_t, c.edi) = GP4_GLOBAL(uint8_t, c.esi);
            c.esi += 1u;
            c.edi += 1u;
            c.ecx -= 1u;
        }
        c.esi = 0x007a9124u;
        c.ecx = Span(0x007a9150u, c.esi);
        while (c.ecx != 0u) {
            GP4_GLOBAL(uint8_t, c.edi) = GP4_GLOBAL(uint8_t, c.esi);
            c.esi += 1u;
            c.edi += 1u;
            c.ecx -= 1u;
        }

        gp4::call_regs(0x005139ed, c);
        gp4::call_regs(0x00520cf4, c);
        f = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00520d54, FUN_00520d54, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=links_0066fef0+ptrs_0066d818+copy_0063b814+save_00520d54+once_00520cf4")

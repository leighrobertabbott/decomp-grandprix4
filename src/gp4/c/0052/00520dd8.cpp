#include <gp4/gp4.h>
#include <gp4/flags.h>

// Byte length of [start, end): kept out of line so MSVC does not fold the two addresses into one
// constant (the original loads both as globals).
static __declspec(noinline) uint32_t Span(uint32_t end, uint32_t start) {
    return end - start;
}

// FUNCTION: GP4 0x00520dd8
// pushal wrapper that restores the game state from the save buffer [0x006975c8] (the inverse of 0x00520d54;
// skipped when it is null, the flags are then those of the null test). It switches the groups off (0x00441a4f),
// turns the car pointers into offsets (0x00513982), copies the word 0x66d814 to 0x679b08 and then reads, in
// order: [0x6975c0] bytes into 0x66d810, the 0xa5e dwords of 0x40b526 (into 0x63b814), [0x7a8e20] blocks of
// [0x6975c4] bytes read contiguously into [0x7a8e18], 0x20 bytes into 0x679b24 and 0x2c bytes into 0x7a9124.
// The word 0x66d8ba is stored to 0x7ad892, offsets become pointers again (0x005139ed) and the control block
// is set up (0x00441a5c). popal restores every register; the flags are those of the last callee.
void __cdecl FUN_00520dd8(gp4::Regs* r) {
    const uint32_t buffer = GP4_GLOBAL(uint32_t, 0x006975c8);
    uint32_t f = gp4::flags::logic32(buffer);
    if (buffer != 0u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00441a4f, c);
        gp4::call_regs(0x00513982, c);
        GP4_GLOBAL(uint32_t, 0x00679b08) = GP4_GLOBAL(uint32_t, 0x0066d814);

        // rep movsd: [0x6975c0] / 4 dwords from the buffer into 0x66d810.
        c.esi = GP4_GLOBAL(uint32_t, 0x006975c8);
        c.edi = 0x0066d810u;
        c.ecx = GP4_GLOBAL(uint32_t, 0x006975c0) >> 2;
        while (c.ecx != 0u) {
            GP4_GLOBAL(uint32_t, c.edi) = GP4_GLOBAL(uint32_t, c.esi);
            c.esi += 4u;
            c.edi += 4u;
            c.ecx -= 1u;
        }
        gp4::call_regs(0x0040b526, c);

        c.esi = c.eax;
        c.edx = 0u;
        c.edi = GP4_GLOBAL(uint32_t, 0x007a8e18);
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

        c.edi = 0x00679b24u;
        c.ecx = Span(0x00679b44u, c.edi);
        while (c.ecx != 0u) {
            GP4_GLOBAL(uint8_t, c.edi) = GP4_GLOBAL(uint8_t, c.esi);
            c.esi += 1u;
            c.edi += 1u;
            c.ecx -= 1u;
        }
        c.edi = 0x007a9124u;
        c.ecx = Span(0x007a9150u, c.edi);
        while (c.ecx != 0u) {
            GP4_GLOBAL(uint8_t, c.edi) = GP4_GLOBAL(uint8_t, c.esi);
            c.esi += 1u;
            c.edi += 1u;
            c.ecx -= 1u;
        }

        GP4_GLOBAL(uint16_t, 0x007ad892) = GP4_GLOBAL(uint16_t, 0x0066d8ba);
        gp4::call_regs(0x005139ed, c);
        gp4::call_regs(0x00441a5c, c);
        f = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00520dd8, FUN_00520dd8, "regs(edx:u32, edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=links_0066fef0+ptrs_0066d818+save_00520d54+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c")

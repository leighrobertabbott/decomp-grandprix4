#include <gp4/gp4.h>

// FUNCTION: GP4 0x00446058
// When bit 0 of [0x62f8bc] is set: raises the four running maxima 0x62f968/6c/70/74 to the matching samples
// 0x62f958/5c/60/64 (signed), marks bit [0x7ab022] of the bitmap 0x62f8e0 when [0x7a8df8] and [0x7a8dfa] are both
// 0xff, clears the bits of the bitmap 0x62f8c8 that are also set in 0x62f8dc, and for those bits retires the cars
// (0x445f75) and re-ranks the slots (0x4456c7); then refreshes the selection flags (0x445a2c, 0x446121).
// [0x7a8e00] is cleared in every case. eax / edx are the registers the original leaves (edx = the previous
// 0x62f8c8, eax from 0x446121); the flags are those 0x446121 leaves, or the failed `test` of the first line.
void __cdecl FUN_00446058(gp4::Regs* r) {
    uint32_t flags;
    if ((GP4_GLOBAL(uint32_t, 0x0062f8bc) & 1u) == 0u) {
        flags = 0x44u;  // test dword, 1 with a zero result: ZF | PF
    } else {
        if (GP4_GLOBAL(int32_t, 0x0062f968) < GP4_GLOBAL(int32_t, 0x0062f958))
            GP4_GLOBAL(int32_t, 0x0062f968) = GP4_GLOBAL(int32_t, 0x0062f958);
        if (GP4_GLOBAL(int32_t, 0x0062f96c) < GP4_GLOBAL(int32_t, 0x0062f95c))
            GP4_GLOBAL(int32_t, 0x0062f96c) = GP4_GLOBAL(int32_t, 0x0062f95c);
        if (GP4_GLOBAL(int32_t, 0x0062f970) < GP4_GLOBAL(int32_t, 0x0062f960))
            GP4_GLOBAL(int32_t, 0x0062f970) = GP4_GLOBAL(int32_t, 0x0062f960);
        if (GP4_GLOBAL(int32_t, 0x0062f974) < GP4_GLOBAL(int32_t, 0x0062f964))
            GP4_GLOBAL(int32_t, 0x0062f974) = GP4_GLOBAL(int32_t, 0x0062f964);

        if (GP4_GLOBAL(uint8_t, 0x007a8df8) == 0xffu && GP4_GLOBAL(uint8_t, 0x007a8dfa) == 0xffu) {
            const uint32_t bit = GP4_GLOBAL(uint8_t, 0x007ab022);
            GP4_ARRAY(uint32_t, 0x0062f8e0)[bit >> 5] |= 1u << (bit & 31u);
        }

        const uint32_t previous = GP4_GLOBAL(uint32_t, 0x0062f8c8);
        const uint32_t retired = GP4_GLOBAL(uint32_t, 0x0062f8dc) & previous;
        GP4_GLOBAL(uint32_t, 0x0062f8c8) = retired ^ previous;

        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.edx = previous;
        c.eax = retired;
        if (retired != 0u) {
            gp4::call_regs(0x00445f75, c);
            gp4::call_regs(0x004456c7, c);
            if (GP4_GLOBAL(uint8_t, 0x0062f8ef) == 0xffu) {
                c.eax = GP4_GLOBAL(uint8_t, 0x007ab022);
            }
        }
        gp4::call_regs(0x00445a2c, c);
        gp4::call_regs(0x00446121, c);

        r->eax = c.eax;
        r->edx = previous;
        flags = c.eflags & 0x8d5u;
    }
    GP4_GLOBAL(uint8_t, 0x007a8e00) = 0;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00446058, FUN_00446058, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+state_00434602+sel_0062f8b0")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054ef46
// Does nothing unless the pending byte 0x838e90 is set. Then (inside a pushal/popal pair, so every
// register the callers care about is unchanged - eax included) it blits the saved 16-bit pixel
// rectangle (eax = y0 [0x838e7c], ebx = x0 [0x838e78], ecx = y1 [0x838e84], ebp = x1 [0x838e80],
// esi = pixels [0x838e88]) with 0x54a12a, inserts it into the sorted pool list with 0x549db9,
// clears the pending byte and, unless the mode byte 0x838e3e is 2, adds [0x838e8c] to the running
// total with 0x538a24 and clears the pixel pointer [0x838e88] and the pending byte again.
// popal does not restore flags: the flags left are those of the last `cmp`, or of 0x538a24.
void __cdecl FUN_0054ef46(gp4::Regs* r) {
    uint32_t flags = gp4::flags::merge(r->eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x00838e90), 0u));
    if (GP4_GLOBAL(uint8_t, 0x00838e90) != 0u) {
        gp4::Regs c = *r;
        c.ebx = GP4_GLOBAL(uint32_t, 0x00838e78);
        c.eax = GP4_GLOBAL(uint32_t, 0x00838e7c);
        c.ebp = GP4_GLOBAL(uint32_t, 0x00838e80);
        c.ecx = GP4_GLOBAL(uint32_t, 0x00838e84);
        c.esi = GP4_GLOBAL(uint32_t, 0x00838e88);
        c.eflags = flags;
        gp4::call_regs(0x0054a12a, c);
        gp4::call_regs(0x00549db9, c);
        GP4_GLOBAL(uint8_t, 0x00838e90) = 0u;
        flags = gp4::flags::merge(c.eflags, gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x00838e3e), 2u));
        if (GP4_GLOBAL(uint8_t, 0x00838e3e) != 2u) {
            c.eflags = flags;
            c.eax = GP4_GLOBAL(uint32_t, 0x00838e8c);
            gp4::call_regs(0x00538a24, c);
            GP4_GLOBAL(uint32_t, 0x00838e88) = 0u;
            GP4_GLOBAL(uint8_t, 0x00838e90) = 0u;
            flags = c.eflags;
        }
    }
    r->eflags = flags;
}
GP4_IMPL(0x0054ef46, FUN_0054ef46, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=surf_0054a12a+list_00549db9+pend_0054ef46+pool_00538a24")

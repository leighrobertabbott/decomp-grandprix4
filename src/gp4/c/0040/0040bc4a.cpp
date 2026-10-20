#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

// Bit length minus one of the value (0 for 0 and 1), counted the way the original does: ebp starts at -1
// and is incremented once per `shr eax, 1` until eax reaches zero (at least one pass).
static inline uint32_t bitLength(uint32_t value) {
    uint32_t count = 0xffffffffu;
    do {
        count += 1u;
        value >>= 1;
    } while (value != 0u);
    return count;
}

// FUNCTION: GP4 0x0040bc4a
// Reads the display/map size record through 0x0051f2d2 (eax = dword [0x0094c004], edx = height << 16 |
// width) and stores it: [0x0062ac9c] = eax, width -> [0x0062aca4], height -> [0x0062aca8], the byte masks
// [0x0062aca0] = ((width - 1) & 0xff) << 8 | ((height - 1) & 0xff), and the bit lengths of width / height
// (position of the top set bit, 0 for 0 and 1) -> [0x0062acac] / [0x0062acb0]. pushal / popal restore every
// register; the flags that survive are those of the last `shr eax, 1` (of the height loop): its final pass
// shifts 0 or 1 down to 0, so ZF and PF are set, CF is the shifted-out bit (set when height != 0).
void __cdecl FUN_0040bc4a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0051f2d2, c);

    GP4_GLOBAL(uint32_t, 0x0062ac9c) = c.eax;
    const uint32_t width = c.edx & 0xffffu;
    GP4_GLOBAL(uint32_t, 0x0062aca4) = width;
    const uint32_t height = c.edx >> 16;
    GP4_GLOBAL(uint32_t, 0x0062aca8) = height;
    GP4_GLOBAL(uint32_t, 0x0062aca0) = ((height - 1u) & 0xffu) | (((width - 1u) & 0xffu) << 8);

    GP4_GLOBAL(uint32_t, 0x0062acac) = bitLength(GP4_GLOBAL(uint32_t, 0x0062aca4));
    GP4_GLOBAL(uint32_t, 0x0062acb0) = bitLength(GP4_GLOBAL(uint32_t, 0x0062aca8));

    r->eflags = (r->eflags & ~kArithFlags) | 0x44u | (height != 0u ? 1u : 0u);
}
GP4_IMPL(0x0040bc4a, FUN_0040bc4a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

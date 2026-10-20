#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t slotPrimary() {
    return 0x3bu;
}

static __declspec(noinline) uint32_t slotMirror() {
    return 0x3du;
}

// FUNCTION: GP4 0x0051f00d
// Stores the time value eax in DAT_00a4b428 and converts it with 0x0051fd6f to packed BCD. The
// character '0' + the digit in bits 12..19 (low byte after shr 12) is written to the cell
// pointed to by table[0x3b] + byte[0x89422c] and to its mirror table[0x3d] + byte[0x894230]
// when it differs from the character already there, and bit 2 of DAT_00852d64 is cleared
// (a redraw flag). Everything runs inside pushal/popal, so only the flags of the last flag
// writer survive: the `cmp al, [esi]` when the cell is up to date, else the final `and`.
void __cdecl FUN_0051f00d(gp4::Regs* r) {
    const uint32_t time = r->eax;

    GP4_GLOBAL(uint32_t, 0x00a4b428) = time;
    uint8_t* cell = reinterpret_cast<uint8_t*>(GP4_ARRAY(uint32_t, 0x00891bd0)[slotPrimary()]) +
                    GP4_GLOBAL(uint8_t, 0x0089422c);
    uint8_t* mirror = reinterpret_cast<uint8_t*>(GP4_ARRAY(uint32_t, 0x00891bd0)[slotMirror()]) +
                      GP4_GLOBAL(uint8_t, 0x00894230);

    gp4::Regs c = *r;
    c.eax = time;
    gp4::call_regs(0x0051fd6f, c);

    const uint32_t digit = static_cast<uint8_t>(((c.eax >> 8) >> 4) + 0x30u);
    uint32_t flags = gp4::flags::sub8(digit, *cell);
    if (digit != *cell) {
        *cell = static_cast<uint8_t>(digit);
        *mirror = static_cast<uint8_t>(digit);
        const uint32_t redraw = GP4_GLOBAL(uint32_t, 0x00852d64) & 0xfffffffbu;
        GP4_GLOBAL(uint32_t, 0x00852d64) = redraw;
        flags = gp4::flags::logic32(redraw);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051f00d, FUN_0051f00d, "regs(eax:u32[0..30000000]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00442d1f")

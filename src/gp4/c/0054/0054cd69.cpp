#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the table of string-start pointers at 0x00835846: the first five entries get the start of each of the
// first five zero-terminated byte-code strings of the stream at 0x00835807 (the skipper 0x0054d0b9 steps esi past
// one string per round); the next 0xb90 entries all get the esi that is left after the fifth string. Exit:
// ecx = 0, esi = that final stream position, edi = one past the last entry; the flags are those of the final
// `dec ecx` (it keeps the CF of the `add edi, 4` before it).
// FUNCTION: GP4 0x0054cd69
void __cdecl FUN_0054cd69(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = 0x00835807u;
    c.edi = 0x00835846u;
    c.ecx = 5u;
    do {
        GP4_GLOBAL(uint32_t, c.edi) = c.esi;
        c.edi += 4u;
        gp4::call_regs(0x0054d0b9, c);
        c.ecx -= 1u;
    } while (c.ecx != 0u);

    uint32_t lastAdd = 0u;
    c.ecx = 0xb90u;
    do {
        GP4_GLOBAL(uint32_t, c.edi) = c.esi;
        lastAdd = gp4::flags::add32(c.edi, 4u);
        c.edi += 4u;
        c.ecx -= 1u;
    } while (c.ecx != 0u);

    r->ecx = 0u;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, lastAdd));
}
GP4_IMPL(0x0054cd69, FUN_0054cd69, "regs() -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Jump-table base addresses of the three switches (the original adds them to selector * 4).


static const uint32_t kTables[3] = {0x00511248u, 0x00511270u, 0x00511298u};

// Selects the carry result of three dispatcher-driven switches. Mode byte 0x007ab020 == 2 asks the
// dispatcher 0x00531f9c for command 0x78 (3-entry table 0x00511298: stc-case, clc, stc-case);
// otherwise the routine returns with CF clear when [0x007a8e0f] is 0 or [0x007ad898] is 5, else asks
// command 0x76 (when [0x007c2ea9] is 0x80; table 0x00511248) or 0x77 (table 0x00511270); both tables
// are {clc, stc-case}. The stc-case 0x005112a6 clears [0x007ad8ae] and sets CF. After the dispatcher
// eax is the clamped selector * 4 + the table address (the `add eax, table` that feeds the jump), and
// the other flags are those of that add; the early exits keep eax and the flags of the last compare
// with CF cleared.
// FUNCTION: GP4 0x005111fc
void __cdecl FUN_005111fc(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t flags;
    uint32_t command;
    uint32_t limit;
    uint32_t clamp;
    uint32_t which;
    bool threeWay;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab020);
    flags = gp4::flags::sub8(mode, 2u);
    if (mode == 2u) {
        command = 0x78u;
        limit = 3u;
        clamp = 2u;
        which = 2u;
        threeWay = true;
    } else {
        const uint8_t active = GP4_GLOBAL(uint8_t, 0x007a8e0f);
        flags = gp4::flags::sub8(active, 0u);
        if (active == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, flags & ~1u);
            return;
        }
        const uint32_t state = GP4_GLOBAL(uint32_t, 0x007ad898);
        flags = gp4::flags::sub32(state, 5u);
        if (state == 5u) {
            r->eflags = gp4::flags::merge(r->eflags, flags & ~1u);
            return;
        }
        const uint8_t kind = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        flags = gp4::flags::sub8(kind, 0x80u);
        if (kind == 0x80u) {
            command = 0x76u;
            which = 0u;
        } else {
            command = 0x77u;
            which = 1u;
        }
        limit = 2u;
        clamp = 1u;
        threeWay = false;
    }

    c.eax = (c.eax & 0xffff0000u) | command;
    c.eflags = gp4::flags::merge(c.eflags, flags);
    gp4::call_regs(0x00531f9c, c);

    uint32_t selector = c.eax;
    if (selector >= limit) {
        selector = clamp;
    }
    const uint32_t table = kTables[which];
    const uint32_t eax = (selector << 2) + table;
    flags = gp4::flags::add32(selector << 2, table);

    const bool carryCase = threeWay ? (selector != 1u) : (selector != 0u);
    if (carryCase) {
        c.eax = eax;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x005112a6, c);
        flags = c.eflags;
    } else {
        flags &= ~1u;
    }

    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005111fc, FUN_005111fc, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

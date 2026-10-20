#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept out of line: MSVC otherwise merges the calls into one call with a selected address, hiding the
// callees from the shape audit.
static __declspec(noinline) void run_005585db(gp4::Regs& c) { gp4::call_regs(0x005585db, c); }
static __declspec(noinline) void run_005584de(gp4::Regs& c) { gp4::call_regs(0x005584de, c); }
static __declspec(noinline) void run_00558557(gp4::Regs& c) { gp4::call_regs(0x00558557, c); }

// Track loader record dispatcher: with the byte [0x00852c38] clear the short 6-word record reader
// 0x005585db runs; otherwise bit 0x20 of the word [0x0086855c] selects the 7-word reader 0x005584de
// (clear) or the 8-word reader 0x00558557 (set). All three leave esi advanced and the flags of their
// final add; every other register is kept.
// FUNCTION: GP4 0x005584b4
void __cdecl FUN_005584b4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t selector = GP4_GLOBAL(uint8_t, 0x00852c38);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(selector, 0u));
    if (selector == 0u) {
        run_005585db(c);
    } else {
        const uint32_t bits = GP4_GLOBAL(uint16_t, 0x0086855c) & 0x20u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic16(bits));
        if (bits == 0u) {
            run_005584de(c);
        } else {
            run_00558557(c);
        }
    }

    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005584b4, FUN_005584b4, "regs(esi:ptr[0x20]:i16[0..200]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+cnt_00870598")

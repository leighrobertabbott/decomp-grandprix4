#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 3 the cursor esi just skips a
// dword. In mode 3 it builds eax from the two words at esi (low word first), masks it with the
// dword at record ebx + 0x2c and hands it to 0x005583a3 with edi = the record (a pushal wrapper, so
// eax stays the masked value and edi is restored). The flags are those left by that routine.
// FUNCTION: GP4 0x005582fb
void __cdecl FUN_005582fb(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 3u) {
        r->esi = esi + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
        return;
    }
    const uint32_t low = GP4_FIELD(uint16_t, esi, 0);
    const uint32_t high = GP4_FIELD(uint16_t, esi, 2);
    const uint32_t bits = ((high << 16) | low) & GP4_FIELD(uint32_t, r->ebx, 0x2c);

    gp4::Regs c = *r;
    c.eax = bits;
    c.edi = r->ebx;
    c.esi = esi + 4u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(bits));
    gp4::call_regs(0x005583a3, c);

    r->eax = c.eax;
    r->esi = c.esi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x005582fb, FUN_005582fb, "regs(eax:u32, ebx:ptr[0x100]:bytes, esi:ptr[8]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")

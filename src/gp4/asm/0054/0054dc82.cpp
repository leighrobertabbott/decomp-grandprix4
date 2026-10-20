#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears bit 7 of the item flag byte [esi + 0x1f]. With CF set on entry it first runs the text-edit
// dialog 0x0054dcad with ebx / eax / ebp / ecx = the words [esi + 4 / 6 / 8 / 0xa] and edi = the dword
// [esi + 0xe] (esi saved and restored around it), then also sets bit 6. The flags are those of the last
// `and`/`or` on the flag byte.
// FUNCTION: GP4 0x0054dc82
void __cdecl FUN_0054dc82(gp4::Regs* r) {
    const uint32_t item = r->esi;

    if ((r->eflags & gp4::CF) == 0u) {
        const uint32_t cleared = GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu;
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(cleared);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint16_t, item, 4);
    c.eax = GP4_FIELD(uint16_t, item, 6);
    c.ebp = GP4_FIELD(uint16_t, item, 8);
    c.ecx = GP4_FIELD(uint16_t, item, 0xa);
    c.edi = GP4_FIELD(uint32_t, item, 0xe);
    gp4::call_regs(0x0054dcad, c);

    const uint32_t cleared = GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu;
    const uint32_t marked = cleared | 0x40u;
    GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(marked);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(marked));
}
GP4_IMPL(0x0054dc82, FUN_0054dc82, "regs(esi:ptr[0x40]:bytes, cf:u32[0..1]) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the key byte *[ebx+4] in the 12-entry table of 0x005341a9 (after the table fill of
// 0x0053414a). On a hit the unaligned dword 0x00828293 is incremented; on a miss eax and the dword
// 0x00828293 are set to [ebx+0xc]. ecx/esi/edi are those the two helpers leave; the flags are the
// lookup's (miss: CF set; hit: the `inc dword` keeps the clear CF).
// FUNCTION: GP4 0x00534f35
void __cdecl FUN_00534f35(gp4::Regs* r) {
    const uint32_t record = r->ebx;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0053414a, c);

    const uint32_t keyPointer = GP4_FIELD(uint32_t, record, 4);
    c.eax = (keyPointer & 0xffffff00u) | GP4_FIELD(uint8_t, keyPointer, 0);
    gp4::call_regs(0x005341a9, c);

    if ((c.eflags & 1u) != 0u) {
        c.eax = GP4_FIELD(uint32_t, record, 0xc);
        GP4_GLOBAL(uint32_t, 0x00828293) = c.eax;
    } else {
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
        GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(before, c.eflags));
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00534f35, FUN_00534f35, "regs(ebx:ptr[0x10]:u32{=0x82829b,0x82829c,0x82829d,0x82829f,0x8282a3,0x8282a7,0x8282ab,0x8282af,0x8282b3,0x8282b7,0x8282c3,0x8282c7,0x828214}) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

// Copies the 22-byte per-car flag table at 0x007c2a48 to 0x007a97b2, clearing the top two bits of every
// byte whose top two bits are both set (`and al, 0x3f`), then latches code 0x10 through 0x0054ac8b
// (eax = 0x10 with ah = the last table byte & 0xc0; esi/edi run past the tables, ecx = 0). Registers and
// flags come back from the callee.
// FUNCTION: GP4 0x0054ad8a
void __cdecl FUN_0054ad8a(gp4::Regs* r) {
    const uint8_t* src = &GP4_GLOBAL(uint8_t, 0x007c2a48);
    uint8_t* dst = &GP4_GLOBAL(uint8_t, 0x007a97b2);
    uint32_t ah = 0u;
    for (uint32_t i = 0; i < 0x16u; ++i) {
        uint32_t al = src[i];
        ah = al & 0xc0u;
        if (ah == 0xc0u) {
            al &= 0x3fu;
        }
        dst[i] = static_cast<uint8_t>(al);
    }

    gp4::Regs c = *r;
    c.esi = 0x007c2a48u + 0x16u;
    c.edi = 0x007a97b2u + 0x16u;
    c.ecx = 0u;
    c.eax = (r->eax & 0xffff0000u) | (ah << 8) | 0x10u;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054ac8b, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054ad8a, FUN_0054ad8a, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043bcd4")

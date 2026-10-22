#include <gp4/gp4.h>
#include <gp4/flags.h>

// When any of the bits 0xfc0 of the mask [0x00828164] is set: stores 0x47 in the dword [0x00828293] and runs the
// front-end poll 0x0053c8af with esi = 0x00852837 (the menu record); registers and flags then come back as the poll
// leaves them. The poll waits through the message pump 0x00509b50 (via 0x0053d350) and so reaches PeekMessageA.
// With none of those bits set only the flags of the `test` change.
// FUNCTION: GP4 0x00534b9a
void __cdecl FUN_00534b9a(gp4::Regs* r) {
    const uint32_t mask = GP4_GLOBAL(uint32_t, 0x00828164) & 0xfc0u;
    if (mask == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(mask));
        return;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(mask));
    GP4_GLOBAL(uint32_t, 0x00828293) = 0x47u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = 0x00852837u;
    gp4::call_regs(0x0053c8af, c);
    *r = c;
}
GP4_IMPL(0x00534b9a, FUN_00534b9a, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")

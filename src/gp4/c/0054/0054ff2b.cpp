#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the byte 0x0083e55c (a 1-based record number) is nonzero: loads al from the first byte of the
// 0x28-byte record 0x0083e55e + (n - 1) * 0x28 (edi = its address), dl = 1, and runs the dispatcher
// 0x0054fd4d. Then posts the window message with 0x00549d12. Registers and flags come back as the
// last callee leaves them.
// FUNCTION: GP4 0x0054ff2b
void __cdecl FUN_0054ff2b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint32_t n = GP4_GLOBAL(uint8_t, 0x0083e55c);
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(n, 0u));
    if (n != 0u) {
        const uint32_t offset = (n - 1u) * 0x28u;
        const uint32_t record = offset + 0x0083e55eu;
        c.edi = record;
        c.eax = (record & 0xffffff00u) | GP4_GLOBAL(uint8_t, record);
        c.edx = (r->edx & 0xffffff00u) | 1u;
        c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x0083e55eu));
        gp4::call_regs(0x0054fd4d, c);
    }
    gp4::call_regs(0x00549d12, c);
    *r = c;
}
GP4_IMPL(0x0054ff2b, FUN_0054ff2b, "regs(eax:u32, edx:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d")

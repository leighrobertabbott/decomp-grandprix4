#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies a text line into the buffer at edi: the string-table entry 0xb8f + (byte [0x7c31dc] & 0x3f)
// is looked up (0x0054d1ad), its first word skipped (0x0052ec92) and up to 0x12 bytes are copied,
// the terminating NUL included (lodsb / stosb / or al,al / loopne). Leaves esi/edi after the last
// byte copied, ecx = 0x12 minus the count, al = the last byte copied; the flags are those of `or al, al`.
// FUNCTION: GP4 0x0052ec6c
void __cdecl FUN_0052ec6c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = (GP4_GLOBAL(uint8_t, 0x007c31dc) & 0x3fu) + 0xb8fu;
    gp4::call_regs(0x0054d1ad, c);
    gp4::call_regs(0x0052ec92, c);

    const uint8_t* src = (const uint8_t*)(uintptr_t)c.esi;
    uint8_t* dst = (uint8_t*)(uintptr_t)r->edi;
    uint32_t count = 0x12u;
    uint32_t ch;
    do {
        ch = *src++;
        *dst++ = (uint8_t)ch;
        count--;
    } while (count != 0u && ch != 0u);

    r->eax = (c.eax & 0xffffff00u) | ch;
    r->ecx = count;
    r->esi = (uint32_t)(uintptr_t)src;
    r->edi = (uint32_t)(uintptr_t)dst;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
}
GP4_IMPL(0x0052ec6c, FUN_0052ec6c, "regs(edi:ptr[0x20]:bytes, eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=strs_0052ec6c")

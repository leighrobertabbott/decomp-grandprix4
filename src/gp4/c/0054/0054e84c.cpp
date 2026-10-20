#include <gp4/gp4.h>

// Frame item wrapper that picks the base value from the settings: edx = 0xdc when [0x7ab0ec] & 3 is not 3,
// or when byte [0x7ab0f2] is 2, or byte [0x7ab17e] is non-zero; 0xd9 otherwise. eax is loaded with
// [0x7ab0ec] & 3 first, then the record at esi is queued by 0x0054e8e7. Every register and the flags come
// back from the callee.
// FUNCTION: GP4 0x0054e84c
void __cdecl FUN_0054e84c(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x007ab0ec) & 3u;
    uint32_t edx = 0xdcu;
    if (mode == 3u) {
        if (GP4_GLOBAL(uint8_t, 0x007ab0f2) != 2u && GP4_GLOBAL(uint8_t, 0x007ab17e) == 0u) {
            edx = 0xd9u;
        }
    }
    gp4::Regs c = *r;
    c.eax = mode;
    c.edx = edx;
    gp4::call_regs(0x0054e8e7, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054e84c, FUN_0054e84c, "regs(esi:ptr[0x40]:u8{0,0,0x10,0x20,0x80,0xa0,0x30,0x90}, ebx:u32, ecx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Conditional frame item wrapper: when the byte switch [0x007aafd3] is nonzero, queues the record at esi
// with 0x0054e8e7 using edx = 0xeb as the base value; otherwise only the flags of `cmp byte, 0` change.
// FUNCTION: GP4 0x0054e8d2
void __cdecl FUN_0054e8d2(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007aafd3);
    if (enabled == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u));
        return;
    }
    gp4::Regs c = *r;
    c.edx = 0xebu;
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
GP4_IMPL(0x0054e8d2, FUN_0054e8d2, "regs(esi:ptr[0x40]:u8{0,0,0x10,0x20,0x80,0xa0,0x30,0x90}, eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")

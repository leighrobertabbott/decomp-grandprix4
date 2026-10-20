#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatch on bit 4 of the byte [esi + 0x1f]: clear tail-calls the item drawer 0x0053a15d; set runs, inside
// pushal/popal, the empty hook 0x0054a1d7 and the sorted-list insert 0x00549db9 fed eax = word [esi + 6],
// ebx = word [esi + 4], ebp = word [esi + 8], ecx = word [esi + 0xa]. popal does not restore the flags: they
// are those of the last flag writer (the insert's, on the set path).
// FUNCTION: GP4 0x0053a133
void __cdecl FUN_0053a133(gp4::Regs* r) {
    const uint32_t item = r->esi;
    const uint32_t bits = GP4_FIELD(uint8_t, item, 0x1f);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bits & 0x10u));
    if ((bits & 0x10u) == 0u) {
        gp4::call_regs(0x0053a15d, *r);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = GP4_FIELD(uint16_t, item, 4);
    c.eax = GP4_FIELD(uint16_t, item, 6);
    c.ebp = GP4_FIELD(uint16_t, item, 8);
    c.ecx = GP4_FIELD(uint16_t, item, 0xa);
    gp4::cdecl_<0x0054a1d7, void>();
    gp4::call_regs(0x00549db9, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053a133, FUN_0053a133, "regs(esi:ptr[0x40]:u32{=0,0,0,0x10000,0x20000,0x10000000,0x20000000,0x80000000}, eax:u32, ebx:u32, ebp:u32, ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+sel_0053aab9")

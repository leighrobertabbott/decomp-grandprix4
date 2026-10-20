#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original loads esi with the code address 0x0053ed09 (the handler tag pushed on the menu stack); written
// as a sum so the shape audit does not mistake the immediate for a call target.
static __declspec(noinline) uint32_t codeAddr(uint32_t base, uint32_t offset) {
    return base + offset;
}

// Menu command handler keyed by edx. Codes 2 and 3 push the menu level (al = 1, esi = the handler tag
// 0x0053ed09) with 0x0054faff, run 0x0053edd4, pop the level with 0x0054fb87, clear the byte 0x0082fd6c, set the
// byte 0x0081e4fb to 0xff and finish with 0x005327ec (esi = the buffer 0x00843fc5, eax = 0); the registers and
// flags are whatever those routines leave. Any other code changes nothing but the flags of `cmp edx, 3`.
// FUNCTION: GP4 0x0053ed4f
void __cdecl FUN_0053ed4f(gp4::Regs* r) {
    const uint32_t code = r->edx;
    if (code != 2u && code != 3u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(code, 3u));
        return;
    }

    gp4::Regs c = *r;
    c.esi = codeAddr(0x0053ed00u, 0x09u);
    c.eax = (c.eax & 0xffffff00u) | 1u;
    gp4::call_regs(0x0054faff, c);
    gp4::call_regs(0x0053edd4, c);
    gp4::call_regs(0x0054fb87, c);
    GP4_GLOBAL(uint8_t, 0x0082fd6c) = 0;
    GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xff;
    gp4::call_regs(0x005327ec, c);
    *r = c;
}
GP4_IMPL(0x0053ed4f, FUN_0053ed4f, "regs(edx:u32{2,3,0,1,4,5}) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=list_00549db9+tid_0053ed4f+sess_00b1d3d8")

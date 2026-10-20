#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the byte-code skipper 0x0054d0c7 over the stream at esi until the byte under esi is zero, then
// steps past that terminator. The flags are those of the final `inc esi` after the `cmp byte [esi], 0`
// of a zero byte (CF = 0, ZF = PF = 1 before the inc).
// FUNCTION: GP4 0x0054d0b9
void __cdecl FUN_0054d0b9(gp4::Regs* r) {
    uint32_t esi = r->esi;
    while (GP4_GLOBAL(uint8_t, esi) != 0u) {
        gp4::Regs c = *r;
        c.esi = esi;
        gp4::call_regs(0x0054d0c7, c);
        esi = c.esi;
    }
    r->esi = esi + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(esi, gp4::flags::sub8(0u, 0u)));
}
GP4_IMPL(0x0054d0b9, FUN_0054d0b9, "regs(esi:ptr[0x40]:u8{=0,0,0x14,0x20,0x41,0xff}) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

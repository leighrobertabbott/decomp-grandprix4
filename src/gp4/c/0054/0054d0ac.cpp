#include <gp4/gp4.h>
#include <gp4/flags.h>

// Repeat wrapper: runs the stream skipper 0x0054d0b9 (esi = byte-code stream) ecx times, advancing esi
// past one zero-terminated entry per call. ecx is pushed around the loop (restored at the end); a zero
// count (jecxz) leaves every register and flag untouched. Flags are those of the last `dec ecx`
// (result zero) with the CF left by the final skipper call.
// FUNCTION: GP4 0x0054d0ac
void __cdecl FUN_0054d0ac(gp4::Regs* r) {
    const uint32_t count = r->ecx;
    if (count == 0u) {
        return;
    }
    gp4::Regs c = *r;
    do {
        gp4::call_regs(0x0054d0b9, c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(c.ecx, c.eflags));
        c.ecx = c.ecx - 1u;
    } while (c.ecx != 0u);
    c.ecx = count;
    *r = c;
}
GP4_IMPL(0x0054d0ac, FUN_0054d0ac, "regs(ecx:u32[0..4], esi:ptr[0x100]:u8{=0,0,0x14,0x20,0x41,0xff}, cf:u32[0..1], pf:u32[0..1], af:u32[0..1], zf:u32[0..1], sf:u32[0..1], of:u32[0..1]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

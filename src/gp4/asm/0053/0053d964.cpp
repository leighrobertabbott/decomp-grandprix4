#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bit-mask toggle: eax &= [0x00827fd4]. A zero result returns at once (eax = 0, flags of the and).
// Otherwise the selected bits are toggled in the state dword 0x00827fd8, the mode byte 0x0082fd45
// is cleared for the duration of the menu refresh 0x0053dc57 and restored afterwards (the original
// pushes eax, whose low byte was loaded from the mode byte, and pops it back: eax leaves as the
// masked value with its low byte replaced by the old mode). The refresh's register results (ecx,
// edx, esi) and flags are kept.
// FUNCTION: GP4 0x0053d964
void __cdecl FUN_0053d964(gp4::Regs* r) {
    const uint32_t masked = r->eax & GP4_GLOBAL(uint32_t, 0x00827fd4);
    r->eax = masked;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(masked));
    if (masked == 0u) {
        return;
    }
    GP4_GLOBAL(uint32_t, 0x00827fd8) ^= masked;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0082fd45);
    const uint32_t saved = (masked & 0xffffff00u) | mode;
    GP4_GLOBAL(uint8_t, 0x0082fd45) = 0;
    r->eax = saved;
    gp4::call_regs(0x0053dc57, *r);
    r->eax = saved;
    GP4_GLOBAL(uint8_t, 0x0082fd45) = mode;
}
GP4_IMPL(0x0053d964, FUN_0053d964, "regs(eax:u32{0,1,2,4,8,0x10,0x20,0x40,0x80,0xffffffff}, ecx:u32, edx:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+items_0053dc57")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of entry eax of the array of 0xf28-byte records at 0x007adb4c. The flags are those of the
// final `add eax, 0x7adb4c`.
// FUNCTION: GP4 0x0052a254
void __cdecl FUN_0052a254(gp4::Regs* r) {
    const uint32_t offset = r->eax * 0xf28u;
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007adb4c)));
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x0052a254, FUN_0052a254, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")

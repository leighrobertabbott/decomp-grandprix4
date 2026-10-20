#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of the 0xe4-byte entry ebx inside entry eax of the array of 0xf28-byte records at
// 0x007adb4c (the entries start 4 bytes in). The flags are those of the final `add eax, 0x7adb4c`.
// ebx is saved and restored.
// FUNCTION: GP4 0x0052a260
void __cdecl FUN_0052a260(gp4::Regs* r) {
    const uint32_t offset = r->eax * 0xf28u + 4u + r->ebx * 0xe4u;
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007adb4c)));
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x0052a260, FUN_0052a260, "regs(eax:u32, ebx:u32) -> (eax, cf, pf, af, zf, sf, of)")

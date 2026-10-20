#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of entry ecx (0x38 bytes) of group ebx (0xe4 bytes, starting 4 bytes into its record)
// inside record eax (0xf28 bytes, entries starting 4 bytes in) of the array at 0x007adb4c. The flags
// are those of the final `add eax, 0x7adb4c`; ebx and ecx are saved and restored.
// FUNCTION: GP4 0x0052a279
void __cdecl FUN_0052a279(gp4::Regs* r) {
    const uint32_t offset = (r->eax * 0xf28u + 4u) + (r->ebx * 0xe4u + 4u) + r->ecx * 0x38u;
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007adb4c)));
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x0052a279, FUN_0052a279, "regs(eax:u32, ebx:u32, ecx:u32) -> (eax, cf, pf, af, zf, sf, of)")

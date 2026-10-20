#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns eax unchanged when it is non-zero, otherwise the dword at 0x008298e0 + 0x14 (a default
// pointer). The flags are those of `or eax, eax` on the incoming eax (the mov does not touch them).
static __declspec(noinline) uint32_t loadDefault(const uint8_t* base) {
    return *reinterpret_cast<const uint32_t*>(base + 0x14);
}

// FUNCTION: GP4 0x005418cd
void __cdecl FUN_005418cd(gp4::Regs* r) {
    const uint32_t value = r->eax;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
    if (value == 0u) {
        r->eax = loadDefault(reinterpret_cast<const uint8_t*>(0x008298e0u));
    }
}
GP4_IMPL(0x005418cd, FUN_005418cd, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

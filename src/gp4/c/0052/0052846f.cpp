#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first free entry (dword +0 == 0) of the 8-entry table at 0x0081cd91 (stride 0x1c, end
// 0x0081ce71): eax = its address, or 0 when every entry is in use. Both exits leave the flags of an
// equal compare (`cmp [eax], 0` / `cmp eax, 0x81ce71`): ZF and PF set, the rest clear.
// FUNCTION: GP4 0x0052846f
void __cdecl FUN_0052846f(gp4::Regs* r) {
    const uint8_t* entry = GP4_ARRAY(uint8_t, 0x0081cd91);
    const uint8_t* const end = GP4_ARRAY(uint8_t, 0x0081ce71);
    uint32_t result = 0u;
    for (; entry != end; entry += 0x1c) {
        if (*reinterpret_cast<const uint32_t*>(entry) == 0u) {
            result = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
            break;
        }
    }
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x0052846f, FUN_0052846f, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=keys_0052848b")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// strlen-like scan of a control-code-terminated string: counts the bytes >= 0x14 starting at esi
// and returns the count in eax (esi unchanged). Flags are those of the final `sub eax, esi`.
// FUNCTION: GP4 0x00546249
void __cdecl FUN_00546249(gp4::Regs* r) {
    const uint32_t start = r->esi;
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(start));
    while (*cursor >= 0x14u) {
        ++cursor;
    }
    const uint32_t end = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eax = end - start;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(end, start));
}
GP4_IMPL(0x00546249, FUN_00546249, "regs(esi:ptr[0x200]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

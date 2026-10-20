#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parser helper (entry jumps to the loop at 0x0054c542): advances the cursor esi over spaces (0x20)
// and tabs (0x09). It returns after `cmp byte [esi], 9` on the first other character, so the flags
// are those of that compare.
// FUNCTION: GP4 0x0054c53f
void __cdecl FUN_0054c53f(gp4::Regs* r) {
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    while (*cursor == 0x20u || *cursor == 0x09u) {
        ++cursor;
    }
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(*cursor, 9u));
}
GP4_IMPL(0x0054c53f, FUN_0054c53f, "regs(esi:ptr[0x40]:u8[0x09..0x21]) -> (esi, cf, pf, af, zf, sf, of)")

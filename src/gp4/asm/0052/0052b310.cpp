#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parser helper: steps the cursor esi over one '=' (0x3d) or ':' (0x3a) separator if present.
// Flags are those of the last instruction executed: the `cmp` against ':' when nothing was skipped,
// otherwise the `inc esi` (CF kept from the preceding compare).
// FUNCTION: GP4 0x0052b310
void __cdecl FUN_0052b310(gp4::Regs* r) {
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t ch = *cursor;
    uint32_t flags = gp4::flags::sub8(ch, 0x3du);
    if (ch != 0x3du) {
        flags = gp4::flags::sub8(ch, 0x3au);
    }
    if (ch == 0x3du || ch == 0x3au) {
        flags = gp4::flags::inc32(r->esi, flags);
        r->esi = r->esi + 1u;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052b310, FUN_0052b310, "regs(esi:ptr[0x10]:u8[0x39..0x3e]) -> (esi, cf, pf, af, zf, sf, of)")

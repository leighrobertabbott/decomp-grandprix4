#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances the cursor esi over bytes that are >= 0x14 (skips printable characters, stops at the
// first control byte or NUL). The flags are those of the final `cmp byte ptr [esi], 0x14`.
// FUNCTION: GP4 0x00546154
void __cdecl FUN_00546154(gp4::Regs* r) {
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(r->esi);
    while (*cursor >= 0x14u) {
        ++cursor;
    }
    r->esi = reinterpret_cast<uint32_t>(cursor);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(*cursor, 0x14u));
}
GP4_IMPL(0x00546154, FUN_00546154, "regs(esi:ptr[0x200]:bytes) -> (esi, cf, pf, af, zf, sf, of)")

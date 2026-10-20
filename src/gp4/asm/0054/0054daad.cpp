#include <gp4/gp4.h>
#include <gp4/flags.h>

// Appends the character in al to a string cursor: stores al at [edi], advances edi by one and
// stores the NUL terminator there. Flags are those of the `inc edi` (CF unchanged).
// FUNCTION: GP4 0x0054daad
void __cdecl FUN_0054daad(gp4::Regs* r) {
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    cursor[0] = static_cast<uint8_t>(r->eax);
    cursor[1] = 0;
    const uint32_t flags = gp4::flags::inc32(r->edi, r->eflags);
    r->edi = r->edi + 1u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054daad, FUN_0054daad, "regs(eax:u32, edi:ptr[0x40]:bytes, cf:u32[0..1]) -> (edi, cf, pf, af, zf, sf, of)")

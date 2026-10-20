#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores the byte in cl through the output cursor edi and advances it (`mov [edi], cl; inc edi; ret`).
// inc keeps the incoming CF and rewrites the other arithmetic flags from the incremented value.
// FUNCTION: GP4 0x0054049d
void __cdecl FUN_0054049d(gp4::Regs* r) {
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    *cursor = static_cast<uint8_t>(r->ecx);
    const uint32_t before = r->edi;
    r->edi = before + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, r->eflags));
}
GP4_IMPL(0x0054049d, FUN_0054049d, "regs(ecx:u32, edi:ptr[0x40]:bytes, cf:u32[0..1]) -> (edi, cf, pf, af, zf, sf, of)")

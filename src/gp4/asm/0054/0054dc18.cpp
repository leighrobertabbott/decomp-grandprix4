#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the three-byte control sequence 07 01 00 at the string cursor edi and leaves edi pointing at
// the last byte written (edi += 2). Flags are those of the `add edi, 2`.
// FUNCTION: GP4 0x0054dc18
void __cdecl FUN_0054dc18(gp4::Regs* r) {
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    cursor[0] = 7;
    cursor[1] = 1;
    cursor[2] = 0;
    const uint32_t flags = gp4::flags::add32(r->edi, 2u);
    r->edi = r->edi + 2u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054dc18, FUN_0054dc18, "regs(edi:ptr[0x40]:bytes) -> (edi, cf, pf, af, zf, sf, of)")

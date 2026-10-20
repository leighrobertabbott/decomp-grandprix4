#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040eb1f
// Tail wrapper: `call 0x0040ea7d ; ret`. The callee compares two records (ebx, esi) and
// answers in CF (clc/stc); every register is preserved by its push/pop frame.
void __cdecl FUN_0040eb1f(gp4::Regs* r) {
    gp4::call_regs(0x0040ea7d, *r);
}
GP4_IMPL(0x0040eb1f, FUN_0040eb1f, "regs(ebx:ptr[0xcc]:bytes, esi:ptr[0xcc]:bytes) -> (cf, pf, af, zf, sf, of)")

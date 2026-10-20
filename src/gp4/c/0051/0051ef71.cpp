#include <gp4/gp4.h>

// FUNCTION: GP4 0x0051ef71
// Tail wrapper: `call 0x0052286e ; ret`. The callee (pushal/popal) bubble-sorts the order list at
// 0x00675958 by the keys at 0x00675970 and preserves every register; the flags are those of its
// closing `test word [0x934df4], 0xffff` (zero result).
void __cdecl FUN_0051ef71(gp4::Regs* r) {
    gp4::call_regs(0x0052286e, *r);
}
GP4_IMPL(0x0051ef71, FUN_0051ef71, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=order_00675958+saved_00934de8+keys_00675970")

#include <gp4/gp4.h>

// `call 0x00509d80 ; ret`: tail wrapper around the compiled routine 0x00509d80 (polls the input device
// object reached through the session pointer [0x00b1d3d8]; returns 0 when it has none). Its only caller
// (0x0053ed4f) goes on to call 0x0054fb87 without reading the flags the callee leaves (its last flag
// writer is the compiled `add esp, 0x158`), so the eax result is what matters.
// FUNCTION: GP4 0x0053edd4
uint32_t __cdecl FUN_0053edd4() {
    gp4::Regs c = {};
    gp4::call_regs(0x00509d80, c);
    return c.eax;
}
GP4_IMPL(0x0053edd4, FUN_0053edd4, "cdecl() -> u32; globals=layout=sess_00b1d3d8")

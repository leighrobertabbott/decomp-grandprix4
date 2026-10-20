#include <gp4/gp4.h>

// Key-queue check: peeks the head of the key queue (0x0052d7c8). An empty queue or a word whose
// high byte is zero gives 0; the word 0x021b gives 1 (and stays queued); any other word is
// consumed (0x0052d7e2) and gives 0. Its only caller tests eax, so no flags are kept.
// FUNCTION: GP4 0x0052d820
uint32_t __cdecl FUN_0052d820() {
    gp4::Regs c = {};
    gp4::call_regs(0x0052d7c8, c);
    if (((c.eax >> 8) & 0xffu) == 0u) {
        return 0u;
    }
    if ((c.eax & 0xffffu) == 0x021bu) {
        return 1u;
    }
    gp4::call_regs(0x0052d7e2, c);
    return 0u;
}
GP4_IMPL(0x0052d820, FUN_0052d820, "cdecl() -> i32; globals=fuzz:i32+layout=keyq_008245c0")

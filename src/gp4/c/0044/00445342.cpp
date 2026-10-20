#include <gp4/gp4.h>

// Same sequence as 0x0044530e / 0x00445328 with the writer 0x00444e03: resets the opcode-stream write cursor
// (0x004453bc), lets 0x00444e03 write its record, copies the stream into the outgoing buffer (0x004453d1),
// dispatches it (0x004452a0) and resets the cursor again. One register block is threaded through all five calls and
// handed back.
// FUNCTION: GP4 0x00445342
void __cdecl FUN_00445342(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x004453bc, state);
    gp4::call_regs(0x00444e03, state);
    gp4::call_regs(0x004453d1, state);
    gp4::call_regs(0x004452a0, state);
    gp4::call_regs(0x004453bc, state);
    *r = state;
}
GP4_IMPL(0x00445342, FUN_00445342, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

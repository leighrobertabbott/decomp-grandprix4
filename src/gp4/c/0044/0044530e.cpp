#include <gp4/gp4.h>

// Resets the opcode-stream write cursor (0x004453bc), lets 0x00444c1c write its records, copies the stream into the
// outgoing buffer (0x004453d1), dispatches it (0x004452a0) and resets the cursor again. Each routine is
// register-convention, so one register block is threaded through all five calls and handed back.
// FUNCTION: GP4 0x0044530e
void __cdecl FUN_0044530e(gp4::Regs* r) {
    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x004453bc, state);
    gp4::call_regs(0x00444c1c, state);
    gp4::call_regs(0x004453d1, state);
    gp4::call_regs(0x004452a0, state);
    gp4::call_regs(0x004453bc, state);
    *r = state;
}
GP4_IMPL(0x0044530e, FUN_0044530e, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041104f+flags_00408f7b+carx_0042fe2f")

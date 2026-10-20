#include <gp4/gp4.h>

// `pushfw ; push eax ; <compare chain> ; pop eax ; popfw ; ret` (66 9c .. 66 9d): the chain only
// compares the word at 0x006f3c80 and the dwords 0x00874fb4, 0x006f2c04 and 0x00679d30; eax and
// the low 16 flag bits (every arithmetic flag) are restored by the pops, so no register, flag or
// memory changes. The reads are dead (lead shape exemption), so the body is empty.
// FUNCTION: GP4 0x00551f83
void __cdecl FUN_00551f83(gp4::Regs*) {
}
GP4_IMPL(0x00551f83, FUN_00551f83, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

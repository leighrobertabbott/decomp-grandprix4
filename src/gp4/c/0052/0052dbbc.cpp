#include <gp4/gp4.h>

// `call 0x0052dbc2 ; ret`: tail wrapper. The callee pops a key word (0x0052d729), looks it up in the
// key-binding tables and, on a hit, writes the key description text through the interpreter
// 0x0054d798; it saves and restores ebx/ecx/edx (and the whole register file around the text
// output), so only eax (the key word) and the flags (CF set by its `stc` / `clc`) come back.
// FUNCTION: GP4 0x0052dbbc
void __cdecl FUN_0052dbbc(gp4::Regs* r) {
    gp4::call_regs(0x0052dbc2, *r);
}
GP4_IMPL(0x0052dbbc, FUN_0052dbbc, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0+script_0054d798+keybindm_0052dbc2")

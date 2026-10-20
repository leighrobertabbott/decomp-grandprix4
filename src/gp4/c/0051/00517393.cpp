#include <gp4/gp4.h>

// FUNCTION: GP4 0x00517393
// Tail wrapper: `call 0x00481020 ; ret`. The callee is compiled C that reads no register: it
// resets the per-car timing record block at 0x00a4b328.. and runs the pass at 0x00517f79. The
// callers (0x005236bb, 0x00523770) overwrite eax and ignore the flags on return.
void __cdecl FUN_00517393() {
    gp4::cdecl_<0x00481020, void>();
}
GP4_IMPL(0x00517393, FUN_00517393, "cdecl() -> void; globals=layout=carbg_0066fef0+carno_0066ff6c+bits_005174cb+order_00675958+keys_006759c8+pitmsg_00517f79+dnames_007c2b3c")

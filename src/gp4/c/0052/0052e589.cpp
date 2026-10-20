#include <gp4/gp4.h>

// `call 0x0052e58f ; ret`: tail wrapper. The callee rebuilds the file-browser listing (drive
// letter [0x81e7f2], SetCurrentDirectoryA / FindFirstFileA / FindNextFileA / FindClose, entries
// appended at [0x826b74], sorted by 0x004fe030); registers and flags come back as it leaves them.
// FUNCTION: GP4 0x0052e589
void __cdecl FUN_0052e589(gp4::Regs* r) {
    gp4::call_regs(0x0052e58f, *r);
}
GP4_IMPL(0x0052e589, FUN_0052e589, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

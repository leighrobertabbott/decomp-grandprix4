#include <gp4/gp4.h>

// Looks up the dword table entry 200 through 0x0054d1ad (esi = table[200]), then calls the
// two-argument routine 0x0054c257 (stdcall, `ret 8`) with the caller's esi and that table value.
// The caller's esi ends up in edi, the table value stays in esi, eax is the call's result.
// FUNCTION: GP4 0x0054e233
void __cdecl FUN_0054e233(gp4::Regs* r) {
    const uint32_t saved = r->esi;

    gp4::Regs c = {};
    c.esi = 200u;
    gp4::call_regs(0x0054d1ad, c);
    const uint32_t entry = c.esi;

    const int result = gp4::stdcall_<0x0054c257, int>(saved, entry);

    r->esi = entry;
    r->edi = saved;
    r->eax = (uint32_t)result;
}
GP4_IMPL(0x0054e233, FUN_0054e233, "regs(esi:u32) -> (eax, esi, edi); globals=fuzz:i32")

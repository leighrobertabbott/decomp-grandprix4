#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042e314
// `push ebx ; mov ebx,[esi+0x114] ; call 0x0042e2f1 ; pop ebx ; ret`: clears the back-reference
// word at +0x1bc (to 0xffff) when it points at the record stored in [esi+0x114]. ebx is
// restored; the flags are those of the callee's last compare.
void __cdecl FUN_0042e314(gp4::Regs* r) {
    const uint32_t savedEbx = r->ebx;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->ebx = GP4_FIELD(uint32_t, record, 0x114);
    gp4::call_regs(0x0042e2f1, *r);
    r->ebx = savedEbx;
}
GP4_IMPL(0x0042e314, FUN_0042e314, "regs(esi:ptr[0x1c4]:bytes) -> (cf, pf, af, zf, sf, of)")

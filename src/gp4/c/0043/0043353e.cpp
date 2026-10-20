#include <gp4/gp4.h>

// edi = a record of 0x118+ bytes. 0x004320b2 builds an x offset in eax from it; the sign of byte
// 0x00868582 picks the side: with the sign set eax = offset - [0x0062d2ac] and the angle term is
// ([0x0062d2b0] << 16) + 0x40000000, otherwise eax = offset + [0x0062d2ac] and the angle term is
// ([0x0062d2b0] << 16) - 0x40000000. The term plus the dword [edi+0xe4] is saved (push edx, popped
// into ecx at the end) and 0x00404796 runs with edx = 0; its eax/edx/flags are the result.
// FUNCTION: GP4 0x0043353e
void __cdecl FUN_0043353e(gp4::Regs* r) {
    const bool leftSide = GP4_GLOBAL(int8_t, 0x00868582) < 0;
    gp4::call_regs(0x004320b2, *r);
    uint32_t term;
    if (leftSide) {
        r->eax = r->eax - GP4_GLOBAL(uint32_t, 0x0062d2ac);
        term = (GP4_GLOBAL(uint32_t, 0x0062d2b0) << 16) + 0x40000000u;
    } else {
        r->eax = r->eax + GP4_GLOBAL(uint32_t, 0x0062d2ac);
        term = (GP4_GLOBAL(uint32_t, 0x0062d2b0) << 16) - 0x40000000u;
    }
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    term += GP4_FIELD(uint32_t, record, 0xe4);
    r->edx = 0u;
    gp4::call_regs(0x00404796, *r);
    r->ecx = term;
}
GP4_IMPL(0x0043353e, FUN_0043353e, "regs(edi:ptr[0x118]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

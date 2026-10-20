#include <gp4/gp4.h>

// Record addressed through esi (only the fields this routine touches).
struct Unknown_0041f1c1_Esi {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x90 - 0x10];
    int16_t Unknown90;
    uint8_t Unknown92[0x240 - 0x92];
    int32_t Result240;
};

// Calls 0x0041f02b with eax = sign-extended word +0x0e and edx = sign-extended word +0x90 of the
// record in esi (edi holds the second record), and stores its result at +0x240.
// FUNCTION: GP4 0x0041f1c1
void __cdecl FUN_0041f1c1(gp4::Regs* r) {
    Unknown_0041f1c1_Esi* rec = reinterpret_cast<Unknown_0041f1c1_Esi*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = static_cast<uint32_t>(static_cast<int32_t>(rec->Unknown0E));
    callee.edx = static_cast<uint32_t>(static_cast<int32_t>(rec->Unknown90));
    gp4::call_regs(0x0041f02b, callee);

    rec->Result240 = static_cast<int32_t>(callee.eax);

    r->eax = callee.eax;
    r->edx = callee.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0041f1c1, FUN_0041f1c1, "regs(esi:ptr[0x248]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=surf_0041f02b")

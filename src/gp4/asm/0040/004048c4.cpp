#include <gp4/gp4.h>

struct Unknown_004048c4_Subject {
    uint8_t Unknown00[0x18];
    uint16_t Unknown18;
    uint8_t Unknown1A[0xa4 - 0x1a];
    int32_t ComponentA4;
    int32_t ComponentA8;
};

// Stores the angle (0x00401315, a table-driven arctangent) of the vector (+0xa4, +0xa8) at +0x18.
// FUNCTION: GP4 0x004048c4
void __cdecl FUN_004048c4(gp4::Regs* r) {
    Unknown_004048c4_Subject* subject = reinterpret_cast<Unknown_004048c4_Subject*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(subject->ComponentA4);
    call.edx = static_cast<uint32_t>(subject->ComponentA8);
    gp4::call_regs(0x00401315, call);
    subject->Unknown18 = static_cast<uint16_t>(call.eax);
    *r = call;
}
GP4_IMPL(0x004048c4, FUN_004048c4, "regs(esi:ptr[0xb0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

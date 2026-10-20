#include <gp4/gp4.h>

struct UnknownRecord_00409213 {
    uint8_t Unknown00[0x73];
    uint8_t Index73;
    uint8_t Unknown74[8];
    uint8_t Key7C;
    uint8_t Unknown7D[0x18f];
    uint8_t Value20C;
};

// Initialises the 0x41c-byte record at esi: clears it (0x00409162), stores al as the key at +0x7c,
// derives the slot byte +0x73 from the key table search (0x00409b17, plus one), fetches the
// parallel table byte into +0x20c (0x00408f1c) and fills the index at +0xfd (0x0043b954).
// eax and ebx are saved and restored; ecx and the flags are those left by the last calls.
// FUNCTION: GP4 0x00409213
void __cdecl FUN_00409213(gp4::Regs* r) {
    UnknownRecord_00409213* record = (UnknownRecord_00409213*)r->esi;
    const uint32_t savedEax = r->eax;
    const uint32_t savedEbx = r->ebx;

    gp4::call_regs(0x00409162, *r);
    record->Key7C = (uint8_t)r->eax;

    gp4::call_regs(0x00409b17, *r);
    r->eax += 1u;
    record->Index73 = (uint8_t)r->eax;

    gp4::call_regs(0x00408f1c, *r);
    record->Value20C = (uint8_t)r->eax;

    gp4::call_regs(0x0043b954, *r);

    r->ebx = savedEbx;
    r->eax = savedEax;
}
GP4_IMPL(0x00409213, FUN_00409213, "regs(eax:u32, esi:ptr[0x41c]:bytes) -> (ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

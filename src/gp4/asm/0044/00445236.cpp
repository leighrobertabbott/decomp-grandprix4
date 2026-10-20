#include <gp4/gp4.h>

struct Unknown_Record {
    uint8_t Unknown00[0x78];
    uint8_t Flags78;
};

// Rewrites bit 7 of the record byte +0x78 from bit 0 of al (clear it, then set it when al & 1), then applies
// the flag byte in al through 0x00445248 (which leaves al masked and the flags of its final `or`).
// FUNCTION: GP4 0x00445236
void __cdecl FUN_00445236(gp4::Regs* r) {
    Unknown_Record* record = reinterpret_cast<Unknown_Record*>(static_cast<uintptr_t>(r->esi));
    record->Flags78 = static_cast<uint8_t>(record->Flags78 & 0x7fu);
    if ((r->eax & 1u) != 0u) {
        record->Flags78 = static_cast<uint8_t>(record->Flags78 | 0x80u);
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00445248, c);
    r->eax = c.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00445236, FUN_00445236, "regs(eax:u32, esi:ptr[0x80]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

// Record addressed through esi (only the fields this routine reads).
struct Unknown_0042453d_Esi {
    uint8_t Unknown00[0x4c];
    uint32_t Unknown4C;
    uint8_t Unknown50[0x6d - 0x50];
    uint8_t Index6D;
};

// pushal ... popal wrapper: runs the 0x0042458b setup for the record in esi, then stores the
// dword at +0x4c into the dword table at 0x006098c0 indexed by the byte at +0x6d. All registers
// are restored by popal; only the flags of the last flag writer inside (the callee) escape.
// FUNCTION: GP4 0x0042453d
void __cdecl FUN_0042453d(gp4::Regs* r) {
    const Unknown_0042453d_Esi* rec = reinterpret_cast<const Unknown_0042453d_Esi*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0042458b, callee);

    const uint32_t index = rec->Index6D;
    const uint32_t value = rec->Unknown4C;
    GP4_ARRAY(uint32_t, 0x006098c0)[index] = value;

    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0042453d, FUN_0042453d, "regs(esi:ptr[0x75]:layout=rec_0042453d) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042458b")

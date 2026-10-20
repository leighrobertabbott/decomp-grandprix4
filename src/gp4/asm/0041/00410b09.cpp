#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410b09
// Refreshes the table entry selected by byte +0x7c of the record in esi through 0x43bb15 (index
// (byte - 1) & 0x3f), then stores byte +0x7f of the record, capped at 5, at +0x32 of the block
// that call leaves in [0x601f24]. eax and ebx are saved and restored and 0x43bb15 is a
// pushal/popal wrapper, so the flags are the ones it leaves.
void __cdecl FUN_00410b09(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t level = *(record + 0x7f);
    if (level > 5u) {
        level = 5u;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = (static_cast<uint32_t>(*(record + 0x7c)) - 1u) & 0x3fu;
    gp4::call_regs(0x0043bb15, call);

    uint8_t* block = GP4_GLOBAL(uint8_t*, 0x00601f24);
    *(block + 0x32) = static_cast<uint8_t>(level);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00410b09, FUN_00410b09, "regs(esi:ptr[0x87]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

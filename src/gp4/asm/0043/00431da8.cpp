#include <gp4/gp4.h>

// FUNCTION: GP4 0x00431da8
// pushal wrapper: a non-negative eax raises the word at +0x60 of the record in esi by 0x40 (at most
// 0x18c0), a negative eax lowers it by 0x40 (at least 0x40), then 0x00408783 refreshes the record.
// popal restores every register but not the flags, which are those 0x00408783 leaves.
void __cdecl FUN_00431da8(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint16_t* word = reinterpret_cast<uint16_t*>(record + 0x60);

    int32_t value = static_cast<int32_t>(*word);
    if (static_cast<int32_t>(r->eax) >= 0) {
        value += 0x40;
        if (value > 0x18c0) {
            value = 0x18c0;
        }
    } else {
        value -= 0x40;
        if (value < 0x40) {
            value = 0x40;
        }
    }
    *word = static_cast<uint16_t>(value);

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(value);
    gp4::call_regs(0x00408783, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00431da8, FUN_00431da8, "regs(eax:i32, esi:ptr[0x1a4]:layout=rec_00431da8) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004087aa+gain_00408790")

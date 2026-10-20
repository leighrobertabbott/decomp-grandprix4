#include <gp4/gp4.h>
#include <gp4/flags.h>

// Car record (esi): when flag byte +0x7b bit 6 is set and the callee 0x004081e6 reports CF=1
// (+0x25f bit 3 clear and +0xe4 == 3), and the record is not the one at [0x00679d2c], the state
// bytes +0x6a (word), +0x6c, +0x6d, +0x6e are cleared and flag byte +0x6f is cleared, keeping only
// its bit 4 when the byte 0x007c2ea9 is not 0x80, the record is [0x00679d30] and bit 4 was set.
// Every exit leaves the flags of its last flag writer (test/call/cmp/or); no register changes.
// FUNCTION: GP4 0x0051d809
void __cdecl FUN_0051d809(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)(uintptr_t)r->esi;

    const uint32_t sel = GP4_FIELD(uint8_t, rec, 0x7b) & 0x40u;
    if (sel == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(sel));
        return;
    }

    gp4::Regs c = *r;
    gp4::call_regs(0x004081e6, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
    if ((c.eflags & 1u) == 0u) {
        return;   // jae
    }

    const uint32_t current = GP4_GLOBAL(uint32_t, 0x00679d2c);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->esi, current));
    if (r->esi == current) {
        return;
    }

    GP4_FIELD(uint16_t, rec, 0x6a) = 0;
    GP4_FIELD(uint8_t, rec, 0x6e) = 0;
    GP4_FIELD(uint8_t, rec, 0x6c) = 0;
    GP4_FIELD(uint8_t, rec, 0x6d) = 0;

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0x80u));
    if (mode == 0x80u) {
        GP4_FIELD(uint8_t, rec, 0x6f) = 0;
        return;
    }

    const uint32_t selected = GP4_GLOBAL(uint32_t, 0x00679d30);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->esi, selected));
    if (r->esi != selected) {
        GP4_FIELD(uint8_t, rec, 0x6f) = 0;
        return;
    }

    const uint32_t bit = GP4_FIELD(uint8_t, rec, 0x6f) & 0x10u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bit));
    if (bit == 0u) {
        GP4_FIELD(uint8_t, rec, 0x6f) = 0;
        return;
    }

    GP4_FIELD(uint8_t, rec, 0x6f) = 0;
    GP4_FIELD(uint8_t, rec, 0x6f) = 0x10;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0x10u));
}
GP4_IMPL(0x0051d809, FUN_0051d809, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=sel_0051634b+cars_004436f9+flags_00432dcf+state_00413f28+sel_0051d809")

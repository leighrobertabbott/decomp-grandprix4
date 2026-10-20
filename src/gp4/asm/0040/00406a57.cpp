#include <gp4/gp4.h>

static inline bool carryOf(const gp4::Regs& r) {
    return (r.eflags & 1u) != 0u;
}

// Searches the ring of 0x160-byte records between [0x00866ed0] and [0x00866ed4] for the first record
// whose classification (0x004067ea, carry = "inside") differs from that of the starting record `edi`
// (clamped into the ring). With carry set the walk goes forward (0x00406a31), otherwise backward
// (0x00406a43); leaving the ring ends it with carry clear. On success the record found is stored in
// [0x00629f10] with its classification value in [0x00629f08], and carry is set.
// FUNCTION: GP4 0x00406a57
void __cdecl FUN_00406a57(gp4::Regs* r) {
    uint32_t record = r->edi;
    if (record < GP4_GLOBAL(uint32_t, 0x00866ed0)) {
        record = GP4_GLOBAL(uint32_t, 0x00866ed0);
    } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ed4)) {
        record = GP4_GLOBAL(uint32_t, 0x00866ed4) - 0x160u;
    }
    r->edi = record;

    gp4::call_regs(0x004067ea, *r);
    if (carryOf(*r)) {
        do {
            GP4_GLOBAL(uint32_t, 0x006278d4) = r->edi;
            GP4_GLOBAL(uint32_t, 0x006278d8) = r->eax;
            gp4::call_regs(0x00406a31, *r);
            if (carryOf(*r)) {
                r->eflags &= ~1u;
                return;
            }
            gp4::call_regs(0x004067ea, *r);
        } while (carryOf(*r));
        r->edi = GP4_GLOBAL(uint32_t, 0x006278d4);
        r->eax = GP4_GLOBAL(uint32_t, 0x006278d8);
    } else {
        do {
            gp4::call_regs(0x00406a43, *r);
            if (carryOf(*r)) {
                r->eflags &= ~1u;
                return;
            }
            gp4::call_regs(0x004067ea, *r);
        } while (!carryOf(*r));
    }
    GP4_GLOBAL(uint32_t, 0x00629f08) = r->eax;
    GP4_GLOBAL(uint32_t, 0x00629f10) = r->edi;
    r->eflags |= 1u;
}
GP4_IMPL(0x00406a57, FUN_00406a57, "regs(edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00406ac7+bounds_00406a57")

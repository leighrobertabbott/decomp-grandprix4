#include <gp4/gp4.h>
#include <gp4/flags.h>

// First record of the track-segment ring (0x160-byte records at 0x008943a0).
static __declspec(noinline) uint32_t RingFirstRecord() {
    return 0x008943a0u;
}

// Attribute-mask smoothing over the track-segment ring. For every record from just before the start of the lap
// (record 0, or the pit start when that is not above it; one record back, wrapped forward by [0x00866ec4] when
// it falls below the lap start [0x00891504]) up to the lap end [0x00866ecc], 0x00558ece computes the record's
// attribute mask (eax). A record clears the bits of its own mask from its dword +0x2c, and when its mask
// shares bits with the previous record's mask those shared bits are cleared from its dwords +0x3c, +0x30,
// +0x34, +0x38 and +0x40. The start of the walk is advanced by one record with the pit/lap wrap rules
// ([0x00891500] pit start, [0x00866ef8] pit end). The routine ends on the compare with the lap end (equal).
// Registers are whatever the callee and the last iteration leave (eax = the shared-bits result, ebx = the last
// mask, esi = the previous mask).
// FUNCTION: GP4 0x00558f50
void __cdecl FUN_00558f50(gp4::Regs* r) {
    gp4::Regs c = *r;

    uint32_t record = RingFirstRecord();
    if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
        record -= 0x160u;
        if (record >= GP4_GLOBAL(uint32_t, 0x00891504)) {
            // stays one record back
        } else {
            record += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else {
        record -= 0x160u;
        if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
            record = GP4_GLOBAL(uint32_t, 0x00891500);
        }
    }

    c.edi = record;
    gp4::call_regs(0x00558ece, c);
    c.esi = c.eax;

    record += 0x160u;
    if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
            record -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }

    do {
        c.edi = record;
        gp4::call_regs(0x00558ece, c);
        c.ebx = c.eax;
        GP4_FIELD(uint32_t, record, 0x2c) &= ~c.eax;
        c.eax = c.ebx & c.esi;
        if (c.eax != 0u) {
            c.eax = ~c.eax;
            GP4_FIELD(uint32_t, record, 0x3c) &= c.eax;
            GP4_FIELD(uint32_t, record, 0x30) &= c.eax;
            GP4_FIELD(uint32_t, record, 0x34) &= c.eax;
            GP4_FIELD(uint32_t, record, 0x38) &= c.eax;
            GP4_FIELD(uint32_t, record, 0x40) &= c.eax;
        }
        c.esi = c.ebx;
        record += 0x160u;
    } while (record != GP4_GLOBAL(uint32_t, 0x00866ecc));

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->esi = c.esi;
    r->edi = record;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(record, record));
}
GP4_IMPL(0x00558f50, FUN_00558f50, "regs() -> (eax, ebx, ecx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0051cc50+attr_00558e7b+ring_00558f50")

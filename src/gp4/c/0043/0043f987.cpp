#include <gp4/gp4.h>

static inline uint8_t segmentFlags(uint32_t record) {
    return GP4_FIELD(uint8_t, record, 0x73);
}

// Adds the 0x0043fab0 figure of the segment record to the running sum.
static inline uint32_t addFigure(gp4::Regs& c, uint32_t record, uint32_t sum) {
    c.edi = record;
    gp4::call_regs(0x0043fab0, c);
    return sum + c.eax;
}

// Walks the 0x160-byte segment records from 0x008943a0 up to the lap end [0x00866ecc], looking for
// groups flagged in byte +0x73: bit 0x40 starts a group (DAT_00627b48), bit 1 closes the first run
// (DAT_00627b50 records, DAT_00603ed0 / DAT_00627b5c = that record, DAT_00627b65 = 0xff when bit 2),
// bit 4 opens an optional second run (up to the next bit 1, unless bit 8 skips its first record;
// DAT_00627b58 records), and bit 0x80 closes the last run (DAT_00627b54 records, DAT_00627b4c,
// DAT_00627b30 = the sum of the 0x0043fab0 figures of every record passed). With the total in
// DAT_0062ac48 the group is handed to 0x0043f696 and the scan goes on from the same record.
static void scanGroups(gp4::Regs& c) {
    uint32_t record = 0x008943a0u;
    for (;;) {
        while ((segmentFlags(record) & 0x40u) == 0) {
            record += 0x160u;
            if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                return;
            }
        }
        GP4_GLOBAL(uint32_t, 0x00627b48) = record;

        uint32_t sum = 0;
        uint32_t count = 0;
        while ((segmentFlags(record) & 1u) == 0) {
            sum = addFigure(c, record, sum);
            ++count;
            record += 0x160u;
            if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                return;
            }
        }
        GP4_GLOBAL(uint32_t, 0x00627b50) = count;
        GP4_GLOBAL(uint32_t, 0x00603ed0) = record;
        GP4_GLOBAL(uint32_t, 0x00627b5c) = record;
        GP4_GLOBAL(uint8_t, 0x00627b65) = 0;
        if ((segmentFlags(record) & 2u) != 0) {
            GP4_GLOBAL(uint8_t, 0x00627b65) = 0xff;
        }
        GP4_GLOBAL(uint32_t, 0x00627b58) = 0;

        if ((segmentFlags(record) & 4u) != 0) {
            count = 0;
            bool skipFirst = (segmentFlags(record) & 8u) != 0;
            for (;;) {
                if (!skipFirst) {
                    if ((segmentFlags(record) & 1u) != 0) {
                        break;
                    }
                    sum = addFigure(c, record, sum);
                }
                skipFirst = false;
                ++count;
                record += 0x160u;
                if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    return;
                }
            }
            GP4_GLOBAL(uint32_t, 0x00627b5c) = record;
            GP4_GLOBAL(uint32_t, 0x00627b58) = count;
        }

        count = 0;
        while ((segmentFlags(record) & 0x80u) == 0) {
            sum = addFigure(c, record, sum);
            ++count;
            record += 0x160u;
            if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                return;
            }
        }
        GP4_GLOBAL(uint32_t, 0x00627b54) = count;
        GP4_GLOBAL(uint32_t, 0x00627b4c) = record;
        GP4_GLOBAL(uint32_t, 0x00627b30) = sum;
        const uint32_t total =
            GP4_GLOBAL(uint32_t, 0x00627b50) + GP4_GLOBAL(uint32_t, 0x00627b58) + GP4_GLOBAL(uint32_t, 0x00627b54);
        GP4_GLOBAL(uint32_t, 0x0062ac48) = total;
        gp4::call_regs(0x0043f696, c);
    }
}

// pushal/popal wrapper: every register survives. The scan only ends through a `cmp edi, [0x866ecc]`
// with equal operands, so the flags are ZF and PF.
// FUNCTION: GP4 0x0043f987
void __cdecl FUN_0043f987(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    scanGroups(c);
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043f987, FUN_0043f987, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043f696+ring_0043faf5+groups_0043f987")

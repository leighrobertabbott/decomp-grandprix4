#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint8_t segmentFlags(uint32_t record) {
    return GP4_FIELD(uint8_t, record, 0x73);
}

// Adds the 0x0043fab0 figure of the segment record to the running sum.
static inline uint32_t addFigure(gp4::Regs& c, uint32_t record, uint32_t sum) {
    c.edi = record;
    gp4::call_regs(0x0043fab0, c);
    return c.eax + sum;
}

static inline uint32_t atLeastOne(uint32_t count) {
    return count != 0u ? count : 1u;
}

// Clears the 32 group entries (0x40 bytes each) at 0x00626fb8, then walks the 0x160-byte segment
// records from 0x008943a0 up to the lap end [0x00866ecc] collecting the groups flagged in byte
// +0x73: bit 0x40 starts a group (DAT_00627b48), the first bit-1 record ends run 1 (DAT_00627b50
// records, figure sum DAT_00627b34; DAT_00603ed0 / DAT_00627b5c = that record; DAT_00627b65 = 0xff
// with bit 2), bit 4 opens run 2 up to the next bit-1 record (bit 8 skips its first record;
// DAT_00627b58, DAT_00627b38), and the first bit-0x80 record ends run 3 (DAT_00627b54,
// DAT_00627b3c, DAT_00627b4c). Every record of the group gets the group number in +0xb6, entry
// `group` of the table receives the bounds, counts, sums and per-run averages, 0x0043fc28 flags
// the records and the scan goes on from the end record, for groups 1..31. The group count goes
// to DAT_006269b4 and 0x0043fcc9 finishes. pushal/popal keep every register; the flags are the
// ones 0x0043fcc9 leaves.
// FUNCTION: GP4 0x0043fd0f
void __cdecl FUN_0043fd0f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint8_t* clear = GP4_ARRAY(uint8_t, 0x00626fb8);
    for (uint32_t i = 0; i < 0x800u; ++i) {
        clear[i] = 0u;
    }

    uint32_t group = 1u;
    uint8_t* entry = GP4_ARRAY(uint8_t, 0x00626fb8);
    uint32_t record = 0x008943a0u;
    for (;;) {
        entry += 0x40;
        while ((segmentFlags(record) & 0x40u) == 0u) {
            record += 0x160u;
            if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                goto done;
            }
        }
        GP4_GLOBAL(uint32_t, 0x00627b48) = record;

        uint32_t sum = 0u;
        uint32_t count = 0u;
        while ((segmentFlags(record) & 1u) == 0u) {
            sum = addFigure(c, record, sum);
            ++count;
            record += 0x160u;
            if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                goto done;
            }
        }
        GP4_GLOBAL(uint32_t, 0x00627b50) = count;
        GP4_GLOBAL(uint32_t, 0x00627b34) = sum;
        GP4_GLOBAL(uint32_t, 0x00603ed0) = record;
        GP4_GLOBAL(uint32_t, 0x00627b5c) = record;
        GP4_GLOBAL(uint8_t, 0x00627b65) = 0u;
        if ((segmentFlags(record) & 2u) != 0u) {
            GP4_GLOBAL(uint8_t, 0x00627b65) = 0xffu;
        }
        GP4_GLOBAL(uint32_t, 0x00627b58) = 0u;
        GP4_GLOBAL(uint32_t, 0x00627b38) = 0u;

        if ((segmentFlags(record) & 4u) != 0u) {
            count = 0u;
            sum = 0u;
            bool skipFirst = (segmentFlags(record) & 8u) != 0u;
            for (;;) {
                if (!skipFirst) {
                    if ((segmentFlags(record) & 1u) != 0u) {
                        break;
                    }
                    sum = addFigure(c, record, sum);
                }
                skipFirst = false;
                ++count;
                record += 0x160u;
                if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    goto done;
                }
            }
            GP4_GLOBAL(uint32_t, 0x00627b5c) = record;
            GP4_GLOBAL(uint32_t, 0x00627b58) = count;
            GP4_GLOBAL(uint32_t, 0x00627b38) = sum;
        }

        count = 0u;
        sum = 0u;
        while ((segmentFlags(record) & 0x80u) == 0u) {
            sum = addFigure(c, record, sum);
            ++count;
            record += 0x160u;
            if (record == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                goto done;
            }
        }
        GP4_GLOBAL(uint32_t, 0x00627b54) = count;
        GP4_GLOBAL(uint32_t, 0x00627b3c) = sum;
        GP4_GLOBAL(uint32_t, 0x00627b4c) = record;
        GP4_GLOBAL(uint32_t, 0x00627b30) =
            GP4_GLOBAL(uint32_t, 0x00627b34) + GP4_GLOBAL(uint32_t, 0x00627b38) + GP4_GLOBAL(uint32_t, 0x00627b3c);
        const uint32_t total =
            GP4_GLOBAL(uint32_t, 0x00627b50) + GP4_GLOBAL(uint32_t, 0x00627b58) + GP4_GLOBAL(uint32_t, 0x00627b54);
        GP4_GLOBAL(uint32_t, 0x0062ac48) = total;

        {
            uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0062ac48);
            uint32_t member = GP4_GLOBAL(uint32_t, 0x00627b48);
            do {
                GP4_FIELD(uint8_t, member, 0xb6) = static_cast<uint8_t>(group);
                member += 0x160u;
            } while (--remaining != 0u);
        }

        GP4_FIELD(uint32_t, entry, 0x08) = GP4_GLOBAL(uint32_t, 0x00627b48);
        GP4_FIELD(uint32_t, entry, 0x00) = GP4_GLOBAL(uint32_t, 0x00603ed0);
        GP4_FIELD(uint32_t, entry, 0x04) = GP4_GLOBAL(uint32_t, 0x00627b5c);
        GP4_FIELD(uint32_t, entry, 0x0c) = GP4_GLOBAL(uint32_t, 0x00627b4c);
        GP4_FIELD(uint32_t, entry, 0x2c) = GP4_GLOBAL(uint32_t, 0x0062ac48);
        GP4_FIELD(uint32_t, entry, 0x30) = GP4_GLOBAL(uint32_t, 0x00627b50);
        GP4_FIELD(uint32_t, entry, 0x34) = GP4_GLOBAL(uint32_t, 0x00627b58);
        GP4_FIELD(uint32_t, entry, 0x38) = GP4_GLOBAL(uint32_t, 0x00627b54);
        GP4_FIELD(uint32_t, entry, 0x10) = GP4_GLOBAL(uint32_t, 0x00627b30);
        GP4_FIELD(uint32_t, entry, 0x14) = GP4_GLOBAL(uint32_t, 0x00627b34);
        GP4_FIELD(uint32_t, entry, 0x18) = GP4_GLOBAL(uint32_t, 0x00627b38);
        GP4_FIELD(uint32_t, entry, 0x1c) = GP4_GLOBAL(uint32_t, 0x00627b3c);
        GP4_FIELD(int32_t, entry, 0x20) = GP4_GLOBAL(int32_t, 0x00627b34)
            / static_cast<int32_t>(atLeastOne(GP4_GLOBAL(uint32_t, 0x00627b50)));
        GP4_FIELD(int32_t, entry, 0x24) = GP4_GLOBAL(int32_t, 0x00627b38)
            / static_cast<int32_t>(atLeastOne(GP4_GLOBAL(uint32_t, 0x00627b58)));
        GP4_FIELD(int32_t, entry, 0x28) = GP4_GLOBAL(int32_t, 0x00627b3c)
            / static_cast<int32_t>(atLeastOne(GP4_GLOBAL(uint32_t, 0x00627b54)));
        if (GP4_GLOBAL(uint8_t, 0x00627b65) != 0u) {
            entry[0x3c] = static_cast<uint8_t>(entry[0x3c] | 1u);
        } else {
            entry[0x3c] = static_cast<uint8_t>(entry[0x3c] & 0xfeu);
        }
        gp4::call_regs(0x0043fc28, c);

        ++group;
        if (static_cast<int32_t>(group) >= 0x20) {
            break;
        }
    }
done:
    GP4_GLOBAL(uint32_t, 0x006269b4) = group;
    gp4::call_regs(0x0043fcc9, c);

    r->eflags = (r->eflags & ~kArithFlags) | (c.eflags & kArithFlags);
}
GP4_IMPL(0x0043fd0f, FUN_0043fd0f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043fcc9+ring_0043f696+ring_0043faf5+groups_0043f987")

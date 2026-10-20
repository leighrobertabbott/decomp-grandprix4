#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `left - right` (cmp).
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline uint32_t wordAt(uint32_t address) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(address));
}

// Moves to the start of the next record: a record whose key dword has bit 31 set is 0xc bytes long,
// otherwise its length is the dword at +4.
static inline uint32_t nextRecord(uint32_t record) {
    return (wordAt(record) & 0x80000000u) != 0u ? record + 0xcu : record + wordAt(record + 4u);
}

// Moves to the start of the previous record: the dword before a record has bit 31 set when the previous
// record is 0xc bytes long, otherwise it holds that record's length.
static inline uint32_t previousRecord(uint32_t record) {
    return (wordAt(record - 4u) & 0x80000000u) != 0u ? record - 0xcu : record - wordAt(record - 4u);
}

// Searches the record list that starts at 0x00630238 (keys ascending, low 31 bits of each record's first
// dword) for the key [0x0066d814]. esi = end of the list, edi = a cursor record (when it is at or past the
// end, the search starts from the record before it). Walks forward while the key is larger than the
// record's, backward while it is smaller. CF = found (edi = the record, eax = the key, edx = its key);
// CF clear leaves the flags of the last compare.
// FUNCTION: GP4 0x0044214c
void __cdecl FUN_0044214c(gp4::Regs* r) {
    const uint32_t listBase = 0x00630238u;
    const uint32_t end = r->esi;
    uint32_t cursor = r->edi;

    if (end == listBase) {
        r->eflags = (r->eflags & ~0x8d5u) | (subtractFlags32(end, listBase) & ~1u);
        return;
    }

    if (cursor >= end) cursor = previousRecord(cursor);

    const uint32_t wanted = GP4_GLOBAL(uint32_t, 0x0066d814);
    uint32_t key = wordAt(cursor) & 0x7fffffffu;
    uint32_t flags;
    bool found = false;

    if (wanted == key) {
        found = true;
        flags = subtractFlags32(wanted, key);
    } else if (wanted > key) {
        for (;;) {
            cursor = nextRecord(cursor);
            if (cursor >= end) {
                flags = subtractFlags32(cursor, end);
                break;
            }
            key = wordAt(cursor) & 0x7fffffffu;
            if (wanted == key) {
                found = true;
                flags = subtractFlags32(wanted, key);
                break;
            }
            if (wanted < key) {
                flags = subtractFlags32(wanted, key);
                break;
            }
        }
    } else {
        for (;;) {
            if (cursor == listBase) {
                flags = subtractFlags32(cursor, listBase);
                break;
            }
            cursor = previousRecord(cursor);
            key = wordAt(cursor) & 0x7fffffffu;
            if (wanted == key) {
                found = true;
                flags = subtractFlags32(wanted, key);
                break;
            }
            if (wanted > key) {
                flags = subtractFlags32(wanted, key);
                break;
            }
        }
    }

    r->eax = wanted;
    r->edx = key;
    r->edi = cursor;
    r->eflags = (r->eflags & ~0x8d5u) | (found ? (flags | 1u) : (flags & ~1u));
}
GP4_IMPL(0x0044214c, FUN_0044214c, "regs(esi:rec[0x630238,0x24,0..10], edi:rec[0x630238,0x24,0..10]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=clock_0066d814+list_0044214c")

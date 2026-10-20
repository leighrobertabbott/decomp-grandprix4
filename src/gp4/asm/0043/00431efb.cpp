#include <gp4/gp4.h>

struct Unknown_0062a970_Entry {
    int32_t Unknown00;       // byte offset added to edx
    uint32_t Unknown04;      // record count
    // followed by Unknown04 sixteen-byte source records
};

struct Unknown_00431efb_Source {
    uint8_t Unknown00[4];
    uint16_t Unknown04;
    uint16_t Unknown06;
    uint16_t Unknown08;
    uint8_t Unknown0A[6];
};
static_assert(sizeof(Unknown_00431efb_Source) == 0x10, "layout");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags (CF PF AF ZF SF OF) of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// eax = index into the pointer table at 0x0062a970, edx = base of an array of 0x160-byte records.
// The entry gives a byte offset (added to edx) and a count; for each of count records the three
// source words (+6, +8, +4 of the 16-byte source records following the 8-byte header) are copied to
// record +0x16, +0x18, +0xa6, the first two negated when byte DAT_00868582 is negative. Leaves
// ecx = 0, edx past the last record, ax = the last +4 word (eax unchanged for an empty entry) and,
// with records, the flags of the final "add edx, 0x160" (else those of "or ecx, ecx").
// FUNCTION: GP4 0x00431efb
void __cdecl FUN_00431efb(gp4::Regs* r) {
    const Unknown_0062a970_Entry* entry = GP4_ARRAY(Unknown_0062a970_Entry*, 0x0062a970)[r->eax];
    uint32_t edx = r->edx + static_cast<uint32_t>(entry->Unknown00);
    uint32_t count = entry->Unknown04;
    uint32_t ax = r->eax & 0xffffu;

    // "or ecx, ecx": CF = OF = AF = 0, ZF/SF/PF from the count.
    uint32_t flags = parityFlag(count)
        | (count == 0u ? 0x40u : 0u)
        | ((count & 0x80000000u) != 0u ? 0x80u : 0u);

    if (count != 0u) {
        const Unknown_00431efb_Source* source = reinterpret_cast<const Unknown_00431efb_Source*>(entry + 1);
        const bool negate = static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0;
        do {
            uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edx));
            uint16_t first = source->Unknown06;
            uint16_t second = source->Unknown08;
            if (negate) {
                first = static_cast<uint16_t>(0u - first);
                second = static_cast<uint16_t>(0u - second);
            }
            *reinterpret_cast<uint16_t*>(record + 0x16) = first;
            *reinterpret_cast<uint16_t*>(record + 0x18) = second;
            ax = source->Unknown04;
            *reinterpret_cast<uint16_t*>(record + 0xa6) = static_cast<uint16_t>(ax);
            source++;
            flags = addFlags32(edx, 0x160u);
            edx += 0x160u;
            count--;
        } while (count != 0u);
    }

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->ecx = 0;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431efb, FUN_00431efb, "regs(eax:u32[0..3], edx:ptr[0x600]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00431efb")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of "cmp left, right" (32-bit).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Record addressed through esi (only the field this routine reads).
struct Unknown_0040edc3_Esi {
    uint8_t Unknown00[0x7c];
    uint8_t Unknown7C;
};

// FUNCTION: GP4 0x0040edc3
// Appends one entry to a 99-entry log (count DAT_007a9124) when logging is enabled
// (byte DAT_00852d98 is zero or has bit 2 set), esi is one of the two records
// DAT_0066d82c (only while byte DAT_007c2e6c is non-zero) / DAT_0066d820, and
// DAT_007a9134 is not -1. Unless bit 2 of DAT_00852d98 is set it stores the byte
// DAT_007a9134, eax and a byte looked up in 0x0066ec4a by the record's 6-bit field at
// +0x7c (1-based, clamped to 0x15). It then copies four dwords to 0x007a9140.. and
// increments the count. ecx leaves holding the count (once read), edx the last copied
// dword on a full pass; the flags are those of the last flag writer on each path.
void __cdecl FUN_0040edc3(gp4::Regs* r) {
    const Unknown_0040edc3_Esi* rec = reinterpret_cast<const Unknown_0040edc3_Esi*>(static_cast<uintptr_t>(r->esi));
    const uint32_t keep = r->eflags & ~0x8d5u;
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00852d98);

    if (mode != 0u && (mode & 4u) == 0u) {
        r->eflags = keep | 0x44u;   // test byte, 4 with a zero result
        return;
    }

    uint32_t flags = 0u;
    bool match = false;
    if (GP4_GLOBAL(uint8_t, 0x007c2e6c) != 0u) {
        const uint32_t other = GP4_GLOBAL(uint32_t, 0x0066d82c);
        flags = subFlags32(r->esi, other);
        match = (r->esi == other);
    }
    if (!match) {
        const uint32_t other = GP4_GLOBAL(uint32_t, 0x0066d820);
        flags = subFlags32(r->esi, other);
        if (r->esi != other) {
            r->eflags = keep | flags;
            return;
        }
    }

    const uint32_t current = GP4_GLOBAL(uint32_t, 0x007a9134);
    flags = subFlags32(current, 0xffffffffu);
    if (current == 0xffffffffu) {
        r->eflags = keep | flags;
        return;
    }

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x007a9124);
    r->ecx = count;
    flags = subFlags32(count, 0x63u);
    if ((int32_t)count >= 0x63) {
        r->eflags = keep | flags;
        return;
    }

    uint32_t carry = 0u;   // test byte, 4 clears CF
    if ((GP4_GLOBAL(uint8_t, 0x00852d98) & 4u) == 0u) {
        GP4_ARRAY(uint8_t, 0x007a92e0)[(int32_t)count] = (uint8_t)GP4_GLOBAL(uint32_t, 0x007a9134);
        GP4_ARRAY(uint32_t, 0x007a9154)[(int32_t)count] = r->eax;
        uint32_t kind = (uint32_t)(rec->Unknown7C & 0x3fu) - 1u;
        carry = (kind < 0x15u) ? 1u : 0u;   // cmp edx, 0x15 (CF survives the inc below)
        if ((int32_t)kind > 0x15) {
            kind = 0x15u;
        }
        GP4_ARRAY(uint8_t, 0x007a9343)[(int32_t)count] = GP4_ARRAY(uint8_t, 0x0066ec4a)[(int32_t)kind];
    }

    GP4_GLOBAL(uint32_t, 0x007a9140) = GP4_GLOBAL(uint32_t, 0x007a9134);
    GP4_GLOBAL(uint32_t, 0x007a9144) = GP4_GLOBAL(uint32_t, 0x007a9138);
    GP4_GLOBAL(uint32_t, 0x007a9148) = GP4_GLOBAL(uint32_t, 0x007a913c);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x0066d834);
    GP4_GLOBAL(uint32_t, 0x007a914c) = last;

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x007a9124);
    const uint32_t after = before + 1u;
    GP4_GLOBAL(uint32_t, 0x007a9124) = after;

    // inc dword [mem]: CF is unchanged, the other flags come from the increment
    flags = carry
        | parityFlag(after)
        | (((before ^ 1u ^ after) & 0x10u) != 0u ? 0x10u : 0u)
        | (after == 0u ? 0x40u : 0u)
        | ((after & 0x80000000u) != 0u ? 0x80u : 0u)
        | (before == 0x7fffffffu ? 0x800u : 0u);

    r->edx = last;
    r->eflags = keep | flags;
}
GP4_IMPL(0x0040edc3, FUN_0040edc3, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x88]:bytes) -> (ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040edc3")

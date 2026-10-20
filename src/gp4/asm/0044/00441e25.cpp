#include <gp4/gp4.h>

// 4-byte entry of the table at 0x0066fd70, indexed by the group number.
struct Unknown_00441e25_Entry {
    uint8_t Flags0;         // bit 7: flag copied to segment bit 6 of +0x48; low 5 bits: record within the group
    uint8_t Selector1;      // set / clear selector for 0x00441d7a
    uint16_t Active2;       // nonzero: the entry is in use
};

// 0x160-byte track-segment record (array at 0x008943a0 / [0x00891504]).
struct Unknown_00441e25_Segment {
    uint8_t Unknown00[0x1e];
    uint16_t Unknown1E;
    uint8_t Unknown20[0x34 - 0x20];
    uint8_t Flags34;
    uint8_t Unknown35[0x38 - 0x35];
    uint8_t Flags38;
    uint8_t Unknown39[0x48 - 0x39];
    uint8_t Flags48;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00441e25
// Switches the track-segment record `(ax << 5) | (entry.Flags0 & 0x1f)` of the group `ax` on (si != 0) or off (si == 0):
// ON stores 2 in word +0x1e, rewrites the bits of +0x48 (bit 6 from entry bit 7, bit 7 set) and sets bit 7 of +0x34 and
// +0x38, then marks the group's 32 records through 0x00441d7a; OFF only when the word is currently 2: clears the same
// bits, zeroes the entry's active word and unmarks through 0x00441d6b. Nothing happens for an unused entry. ecx, ebx,
// edx (low word), eax and edi are left as the code computes them (callers wrap it in pushal / popal); the flags are
// those of the last compare or of the callee.
void __cdecl FUN_00441e25(gp4::Regs* r) {
    const uint16_t group = static_cast<uint16_t>(r->eax);
    const int32_t entryOffset = static_cast<int16_t>(static_cast<uint16_t>(group << 2));
    r->ecx = static_cast<uint32_t>(entryOffset);

    uint8_t* entryBytes = &GP4_ARRAY(uint8_t, 0x0066fd70)[entryOffset];
    Unknown_00441e25_Entry* entry = reinterpret_cast<Unknown_00441e25_Entry*>(entryBytes);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entryBytes));

    if (entry->Active2 == 0u) {
        r->edx = (r->edx & 0xffff0000u) | group;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    const uint16_t recordIndex = static_cast<uint16_t>((group << 5) | (entry->Flags0 & 0x1fu));
    const uint32_t offset = 0x160u * recordIndex;
    r->eax = offset;
    r->edx = (r->edx & 0xffff0000u) | (offset >> 16);
    r->edi = GP4_GLOBAL(uint32_t, 0x00891504) + offset;

    Unknown_00441e25_Segment* segment = reinterpret_cast<Unknown_00441e25_Segment*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint32_t flags;

    if (static_cast<uint16_t>(r->esi) != 0u) {
        segment->Unknown1E = 2u;
        segment->Flags48 = static_cast<uint8_t>(segment->Flags48 & 0xbfu);
        if ((entry->Flags0 & 0x80u) != 0u) {
            segment->Flags48 = static_cast<uint8_t>(segment->Flags48 | 0x40u);
        }
        segment->Flags48 = static_cast<uint8_t>(segment->Flags48 | 0x80u);
        segment->Flags34 = static_cast<uint8_t>(segment->Flags34 | 0x80u);
        segment->Flags38 = static_cast<uint8_t>(segment->Flags38 | 0x80u);
        gp4::call_regs(0x00441d7a, c);
        flags = c.eflags;
    } else if (segment->Unknown1E != 2u) {
        flags = cmpFlags16(segment->Unknown1E, 2u);
    } else {
        segment->Unknown1E = 0u;
        segment->Flags48 = static_cast<uint8_t>(segment->Flags48 & 0xbfu);
        segment->Flags48 = static_cast<uint8_t>(segment->Flags48 & 0x7fu);
        segment->Flags34 = static_cast<uint8_t>(segment->Flags34 & 0x7fu);
        segment->Flags38 = static_cast<uint8_t>(segment->Flags38 & 0x7fu);
        entry->Active2 = 0u;
        gp4::call_regs(0x00441d6b, c);
        flags = c.eflags;
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00441e25, FUN_00441e25, "regs(eax:u32[0..1], edx:u32, esi:u32[0..1], edi:u32) -> (eax, ecx, edx, ebx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `cmp left, right` (also `neg x` as cmp 0, x).
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

struct Unknown_00431ffb_Record {
    uint8_t Unknown00[0x7e];
    uint8_t Selector7E;
    uint8_t Unknown7F[0x26b - 0x7f];
    uint8_t Flags26B;
};

struct Unknown_00431ffb_Segment {
    uint32_t Start;
    uint32_t Count;
    uint8_t Entries[1];   // Count entries of 0x10 bytes, a signed word at +6
};

// Chooses the word to use for the 0x160-byte slot at edi. With a selector at esi+0x7e, bit 7 of
// esi+0x26b and a segment table entry (0x62a970[selector]) with a non-zero count, the slot index
// is (edi - (0x43207c + 0x432c03 base + Start)) / 0x160 when edi lies inside the segment, and the
// result is the word at +6 of that 16-byte entry, negated when the byte at 0x868582 has bit 7.
// Otherwise (no segment, or edi outside) it is the signed word at edi+0x16. All registers except
// eax are restored; the flags are those of the last compare/test.
// FUNCTION: GP4 0x00431ffb
void __cdecl FUN_00431ffb(gp4::Regs* r) {
    const Unknown_00431ffb_Record* record =
        reinterpret_cast<const Unknown_00431ffb_Record*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* slot = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t position = r->edi;

    if (record->Selector7E == 0u || (record->Flags26B & 0x80u) == 0u) {
        r->eax = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(slot + 0x16)));
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    const Unknown_00431ffb_Segment* segment = reinterpret_cast<const Unknown_00431ffb_Segment*>(
        static_cast<uintptr_t>(GP4_ARRAY(uint32_t, 0x0062a970)[record->Selector7E]));
    if (segment->Count == 0u) {
        r->eax = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(slot + 0x16)));
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0043207c, callee);
    gp4::call_regs(0x00432c03, callee);
    const uint32_t start = callee.eax + segment->Start;

    uint32_t flags = subFlags(position, start);
    if (position < start) {
        r->eax = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(slot + 0x16)));
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }
    const uint32_t end = segment->Count * 0x160u + start;
    flags = subFlags(position, end);
    if (position >= end) {
        r->eax = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(slot + 0x16)));
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    int32_t remainder;
    const int32_t index = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(position - start)), 0x160, &remainder);
    uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(
        *reinterpret_cast<const int16_t*>(segment->Entries + static_cast<uint32_t>(index) * 0x10u + 6u)));

    const uint32_t discriminator = GP4_GLOBAL(uint8_t, 0x00868582);
    if ((discriminator & 0x80u) != 0u) {
        flags = subFlags(0u, value);   // neg eax
        value = 0u - value;
    } else {
        flags = parityFlag(discriminator) | (discriminator == 0u ? 0x40u : 0u);
    }
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00431ffb, FUN_00431ffb, "regs(esi:ptr[0x273]:layout=car_00431ffb, edi:rec[0x8943a0,0x160,0..17]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=words_008943a0+state_00431ffb")

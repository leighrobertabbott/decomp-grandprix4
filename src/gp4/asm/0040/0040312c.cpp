#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 16-bit `left - right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag8(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// Arithmetic flags of the 16-bit `left + right`.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    const uint32_t sum = left + right;
    const uint32_t result = sum & 0xffffu;
    return (sum > 0xffffu ? 1u : 0u) | parityFlag8(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) >> 4);
}

// ax = word[+0x160] - word[+0] of the segment record in edi (the record linked at +0x44 when
// esi's flag 0x9c&2 is set and edi lies below DAT_00891500). Without the esi flags 0x9c&2 / 0x8d&2
// it adds the interpolated difference (word[+0x178] - word[+0x18]) * dword[esi+0x1a0] >> 14,
// leaving cx = the first difference and edx = the high half of the product.
// FUNCTION: GP4 0x0040312c
void __cdecl FUN_0040312c(gp4::Regs* r) {
    uint8_t* state = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    bool linked = false;
    if ((state[0x9c] & 2u) != 0u) {
        if (r->edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
            linked = true;
        } else {
            const uint32_t link = GP4_FIELD(uint32_t, record, 0x44);
            if (link != 0u) {
                record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(link));
                linked = true;
            }
        }
    }

    const uint32_t left = GP4_FIELD(uint16_t, record, 0x160);
    const uint32_t right = GP4_FIELD(uint16_t, record, 0x00);
    const uint32_t diff = (left - right) & 0xffffu;
    uint32_t flags = subFlags16(left, right);
    r->eax = (r->eax & 0xffff0000u) | diff;

    if (!linked && (state[0x8d] & 2u) == 0u) {
        const uint32_t adjustLeft = GP4_FIELD(uint16_t, record, 0x178);
        const uint32_t adjustRight = GP4_FIELD(uint16_t, record, 0x18);
        const int32_t span = static_cast<int16_t>((adjustLeft - adjustRight) & 0xffffu);
        const int32_t factor = GP4_FIELD(int32_t, state, 0x1a0);
        const int64_t product = static_cast<int64_t>(span) * static_cast<int64_t>(factor);
        const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        const uint32_t sum = ((shifted & 0xffffu) + diff) & 0xffffu;
        flags = addFlags16(shifted & 0xffffu, diff);
        r->eax = (shifted & 0xffff0000u) | sum;
        r->ecx = (r->ecx & 0xffff0000u) | diff;
        r->edx = high;
    } else if (!linked) {
        // test byte [esi+0x8d], 2 with the bit set: every arithmetic flag clear
        flags = 0u;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040312c, FUN_0040312c, "regs(eax:u32, ecx:u32, esi:ptr[0x1a8]:bytes, edi:rec[0x8943a0,0x160,0..24]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=links_00405a12+state_00432330")

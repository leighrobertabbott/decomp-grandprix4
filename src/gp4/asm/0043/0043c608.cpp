#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `cmp left, right`.
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t flags = parityFlag(result);
    if (left < right) flags |= 0x001u;
    if (((left ^ right ^ result) & 0x10u) != 0u) flags |= 0x010u;
    if (result == 0u) flags |= 0x040u;
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

// FUNCTION: GP4 0x0043c608
// Walks the records from the pointer [esi+0x1c] (step 0x1600) while below [0x00866f00], provided it
// starts below both [0x00891500] and [0x00866f00]. Each record contributes
// (0xfa0000 / max(word +0xca, 0x780)) * (16 * table[10 * word +0x7a + 1] + 0x4000) >> 14,
// the table being the byte array at 0x00675a82. Returns the sum in eax (0 when the walk does not
// start); ecx = the sum, edx = high half of the last product, ebp = the last divisor and edi = the
// final record pointer. ebx is saved and restored. The flags are those of the last compare.
void __cdecl FUN_0043c608(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t edi = GP4_FIELD(uint32_t, self, 0x1c);

    const uint32_t firstLimit = GP4_GLOBAL(uint32_t, 0x00891500);
    if (edi >= firstLimit) {
        r->eax = 0u;
        r->edi = edi;
        r->eflags = (r->eflags & ~0x8d5u) | subFlags(edi, firstLimit);
        return;
    }
    const uint32_t secondLimit = GP4_GLOBAL(uint32_t, 0x00866f00);
    if (edi >= secondLimit) {
        r->eax = 0u;
        r->edi = edi;
        r->eflags = (r->eflags & ~0x8d5u) | subFlags(edi, secondLimit);
        return;
    }

    uint32_t sum = 0u;
    uint32_t divisor;
    uint32_t high;
    uint32_t limit;
    do {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi));
        divisor = GP4_FIELD(uint16_t, record, 0xca);
        if (divisor < 0x780u) {
            divisor = 0x780u;
        }
        const int32_t quotient = static_cast<int32_t>(0xfa0000u / divisor);
        const uint32_t index = static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x7a)) * 10u;
        const int32_t factor = static_cast<int32_t>((static_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x00675a82)[index + 1u]) << 4) + 0x4000u);
        const int64_t product = static_cast<int64_t>(quotient) * static_cast<int64_t>(factor);
        const uint32_t low = static_cast<uint32_t>(product);
        high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        sum += (low >> 14) | (high << 18);
        edi += 0x1600u;
        limit = GP4_GLOBAL(uint32_t, 0x00866f00);
    } while (edi < limit);

    r->eax = sum;
    r->ecx = sum;
    r->edx = high;
    r->ebp = divisor;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(edi, limit);
}
GP4_IMPL(0x0043c608, FUN_0043c608, "regs(esi:u32[0x66fef0..0x66fef0]) -> (eax, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=state_00432330+segw_0043c608")

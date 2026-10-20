#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right on bytes
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Fills one row of the word table at 0x007c28bc (16 bytes per row, row chosen by the clamped
// (body[+0x7c] - 1) & 0x3f) for the columns 2..body[+0x1cd]: each word is
// f(col) * (DAT_0060131c << 14) / max(f(col - 1), 1) >> 14 minus DAT_00601324, where f is the
// per-index lookup 0x0041242f on the table at body+0x1c6. pushal/popal keep every register; the
// flags are those of the final "cmp cl, [esi+0x1cd]".
// FUNCTION: GP4 0x0041043d
void __cdecl FUN_0041043d(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t row = static_cast<uint32_t>(static_cast<uint8_t>(GP4_FIELD(uint8_t, body, 0x7c) - 1u)) & 0x3fu;
    if (static_cast<int32_t>(row) > 0x15) {
        row = 0x15u;
    }
    const uint32_t rowOffset = row << 4;

    uint32_t column = 2u;
    uint32_t flags;
    for (;;) {
        gp4::Regs lower = *r;
        lower.eax = (column - 1u) & 0xffu;
        lower.edx = r->esi + 0x1c6u;
        gp4::call_regs(0x0041242f, lower);
        int32_t divisor = static_cast<int32_t>(lower.eax);
        if (divisor == 0) {
            divisor = 1;
        }
        const int32_t dividend = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0060131c) << 14);
        int32_t remainder;
        const int32_t ratio = gp4::idiv64(static_cast<int64_t>(dividend), divisor, &remainder);

        gp4::Regs upper = *r;
        upper.eax = column & 0xffu;
        upper.edx = r->esi + 0x1c6u;
        gp4::call_regs(0x0041242f, upper);
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(upper.eax)) * ratio;
        const uint32_t shifted = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        const uint32_t value = shifted - GP4_GLOBAL(uint32_t, 0x00601324);

        *reinterpret_cast<uint16_t*>(GP4_ARRAY(uint8_t, 0x007c28bc) + rowOffset + column * 2u) =
            static_cast<uint16_t>(value);

        ++column;
        const uint32_t limit = GP4_FIELD(uint8_t, body, 0x1cd);
        flags = cmpFlags8(column & 0xffu, limit);
        if (!(static_cast<int8_t>(column) <= static_cast<int8_t>(limit))) {
            break;
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041043d, FUN_0041043d, "regs(esi:ptr[0x250]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// ZF, SF, PF of a 32-bit result.
static inline uint32_t resultFlags(uint32_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Sets the word at car +0xa2 to (curve value * word [0x0062d1fa]) >> 14, where the curve value is the first dword
// of the 0x30-byte row 0x00602dec + 0x30 * [0x0062d1d8 + 4 * byte [0x007ad9fc]]. eax / ebx / edx are left as the
// imul and `shrd eax, edx, 14` leave them (ebx = row offset, edx = high half of the product).
// FUNCTION: GP4 0x0044138b
void __cdecl FUN_0044138b(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t level = GP4_GLOBAL(uint8_t, 0x007ad9fc);
    const uint32_t row = GP4_ARRAY(const uint32_t, 0x0062d1d8)[level];
    const uint32_t offset = row * 0x30u;
    const int32_t curve = *reinterpret_cast<const int32_t*>(GP4_ARRAY(const uint8_t, 0x00602dec) + offset);
    const uint32_t scale = GP4_GLOBAL(uint16_t, 0x0062d1fa);

    const int64_t product = static_cast<int64_t>(curve) * static_cast<int64_t>(scale);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    *reinterpret_cast<uint16_t*>(car + 0xa2) = static_cast<uint16_t>(result);

    // shrd eax, edx, 14: CF = last bit out, AF = 0 (pinned model), OF = bit31(prev ^ result)
    const uint32_t prev = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    uint32_t flags = resultFlags(result);
    if ((prev & 1u) != 0u) flags |= 0x01u;
    if (((prev ^ result) & 0x80000000u) != 0u) flags |= 0x800u;

    r->eax = result;
    r->ebx = offset;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044138b, FUN_0044138b, "regs(esi:ptr[0xaa]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0043c1ba")

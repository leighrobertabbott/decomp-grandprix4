#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_008943a0 { uint8_t Unknown00[0x2c]; uint32_t Flags2C; uint8_t Unknown30[0x130]; };

// FUNCTION: GP4 0x0040ad7c
void __cdecl FUN_0040ad7c(gp4::Regs* r) {
    const uint32_t mask = GP4_GLOBAL(int8_t, 0x00868582) < 0 ? 0x20u : 0x10u;
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866f04);
    uint32_t record = 0x008943a0u + 0x580u;
    for (;;) {
        const Unknown_008943a0* entry = reinterpret_cast<const Unknown_008943a0*>(static_cast<uintptr_t>(record));
        if ((entry->Flags2C & mask) == 0u) break;
        record += 0x160u;
        if (record == end) break;
    }
    r->eax = record - 0x160u;
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags32(record, 0x160u);
}
GP4_IMPL(0x0040ad7c, FUN_0040ad7c, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=records_008943a0")

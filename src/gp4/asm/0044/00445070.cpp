#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Appends one 10-byte record to the byte stream whose write cursor is the pointer at 0x0062d1a0.
// FUNCTION: GP4 0x00445070
void __cdecl FUN_00445070(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0)));

    cursor[0] = 0x0e;
    const uint32_t first = *reinterpret_cast<const uint32_t*>(record + 0xf0);
    *reinterpret_cast<uint32_t*>(cursor + 1) = first;
    const uint32_t second = *reinterpret_cast<const uint32_t*>(record + 0x1d8);
    *reinterpret_cast<uint32_t*>(cursor + 5) = second;
    uint8_t flag = 0;
    if ((record[0x8d] & 0x20) != 0) {
        flag = 1;
    }
    cursor[9] = flag;

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 0x0au;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 0x0au);
}
GP4_IMPL(0x00445070, FUN_00445070, "regs(esi:ptr[0x1e8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0")

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

// Schedules a due time on the object in esi: two bytes from the random generator 0x00401462 give a
// triangular deviation |r1 + r2 - 256| (0..256), scaled through the pair table at 0x006029f0 by
// the object's byte +0x84 (low word of the product plus the entry selected by the high word), halved
// when byte +0x87 has bit 2, and added to the clock [0x0066d814] into dword +0x1e4. edx is left
// holding the high half of the product; flags are those of the final add into memory.
// FUNCTION: GP4 0x00440d89
void __cdecl FUN_00440d89(gp4::Regs* r) {
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401462, call);
    const uint32_t first = call.eax & 0xffu;
    call.eax = first;
    gp4::call_regs(0x00401462, call);
    const uint32_t second = call.eax & 0xffu;

    int32_t deviation = static_cast<int32_t>(second + first) - 0x100;
    if (deviation < 0) {
        deviation = -deviation;
    }

    const int32_t scale = GP4_ARRAY(int32_t, 0x006029f0)[static_cast<uint32_t>(object[0x84]) * 2u];
    const int64_t product = static_cast<int64_t>(deviation) * static_cast<int64_t>(scale);
    const uint32_t low = static_cast<uint32_t>(product);
    const int32_t high = static_cast<int32_t>(product >> 32);
    uint32_t result = low + GP4_ARRAY(uint32_t, 0x006029f4)[static_cast<uint32_t>(high) * 2u];
    if ((object[0x87] & 4u) != 0u) {
        result >>= 1;
    }

    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    *reinterpret_cast<uint32_t*>(object + 0x1e4) = clock;
    *reinterpret_cast<uint32_t*>(object + 0x1e4) = clock + result;

    r->eax = result;
    r->edx = static_cast<uint32_t>(high);
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(clock, result);
}
GP4_IMPL(0x00440d89, FUN_00440d89, "regs(eax:u32, esi:ptr[0x1ec]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0")

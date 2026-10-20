#include <gp4/gp4.h>
#include <gp4/flags.h>

// Points the four table-pointer globals at their data (0x008914ec -> 0x006f3fb0, 0x008914f0 ->
// 0x006f6ddc, 0x008914f4 -> 0x006f4dc0, 0x00876f78 -> 0x00852e04) and stores [0x0087549c] << 3 in
// 0x008754a0. eax is left as that product; the flags are those of the final `shl eax, 3`.

// FUNCTION: GP4 0x00551848
void __cdecl FUN_00551848(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x008914ec) = 0x006f3fb0u;
    GP4_GLOBAL(uint32_t, 0x008914f0) = 0x006f6ddcu;
    GP4_GLOBAL(uint32_t, 0x008914f4) = 0x006f4dc0u;
    GP4_GLOBAL(uint32_t, 0x00876f78) = 0x00852e04u;
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x0087549c);
    const uint32_t result = value << 3;
    GP4_GLOBAL(uint32_t, 0x008754a0) = result;

    // shl by 3: CF = last bit shifted out, AF = 0, OF = bit31(value << 2 ^ result).
    uint32_t arith = gp4::flags::szp(result, 32);
    arith |= (value >> 29) & 1u;
    arith |= ((((value << 2) ^ result) >> 31) & 1u) << 11;
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00551848, FUN_00551848, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

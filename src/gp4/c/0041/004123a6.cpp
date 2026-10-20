#include <gp4/gp4.h>

// DAT_00602b4c = (int)((0x10000000 / DAT_00602a40) * (int64)DAT_00602a48 >> 28), i.e. a 4.28
// fixed-point reciprocal times a factor, shrunk back by 28 bits (div, imul, shrd by 0x1c).
// FUNCTION: GP4 0x004123a6
void __cdecl FUN_004123a6() {
    uint32_t rem;
    const uint32_t quotient = gp4::div64(0x10000000ull, GP4_GLOBAL(uint32_t, 0x00602a40), &rem);
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(quotient)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00602a48));
    GP4_GLOBAL(uint32_t, 0x00602b4c) = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 28);
}
GP4_IMPL(0x004123a6, FUN_004123a6, "cdecl() -> void; globals=layout=state_004123a6")

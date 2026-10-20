#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0041774e
// With ebx = 0x006380d8 (left holding that value): stores (dword [ebx+0x2c4] >> 10) as a word at
// [esi+0x27e] and (dword [ebx+0x8c4] >> 10) as a word at [esi+0x278]. eax is the last shifted value;
// the flags are those of the last "sar eax, 10".
void __cdecl FUN_0041774e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    r->ebx = 0x006380d8u;
    const int32_t first = *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(r->ebx) + 0x2c4);
    *reinterpret_cast<int16_t*>(record + 0x27e) = (int16_t)(first >> 10);

    const int32_t second = *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(r->ebx) + 0x8c4);
    const int32_t shifted = second >> 10;
    *reinterpret_cast<int16_t*>(record + 0x278) = (int16_t)shifted;

    const uint32_t result = (uint32_t)shifted;
    const uint32_t flags = (((uint32_t)second >> 9) & 1u) | parityFlag(result)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041774e, FUN_0041774e, "regs(esi:ptr[0x286]:bytes) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=integ_004200a7")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Loads the 0x78-byte entry (table at 0x0063bd7c, index = byte +0xdc of the record in eax) into
// the record: three words >> 5 to +0x18/+0x1c/+0x20, three words to +0xc0/+0xc4/+0xc8, and the
// entry's +0x64 offset rebased on 0x008943a0 to +0x84. eax/ebx/esi are restored; the flags are
// those of the final "add eax, 0x8943a0".
// FUNCTION: GP4 0x00424371
void __cdecl FUN_00424371(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->eax));
    const uint8_t index = GP4_FIELD(uint8_t, record, 0xdc);
    const uint32_t entry = 0x0063bd7cu + static_cast<uint32_t>(index) * 0x78u;

    GP4_FIELD(int32_t, record, 0x18) = GP4_GLOBAL(int32_t, entry) >> 5;
    GP4_FIELD(int32_t, record, 0x1c) = GP4_GLOBAL(int32_t, entry + 0x04u) >> 5;
    GP4_FIELD(int32_t, record, 0x20) = GP4_GLOBAL(int32_t, entry + 0x08u) >> 5;
    GP4_FIELD(uint32_t, record, 0xc0) = GP4_GLOBAL(uint32_t, entry + 0x0cu);
    GP4_FIELD(uint32_t, record, 0xc4) = GP4_GLOBAL(uint32_t, entry + 0x10u);
    GP4_FIELD(uint32_t, record, 0xc8) = GP4_GLOBAL(uint32_t, entry + 0x14u);

    const uint32_t offset = GP4_GLOBAL(uint32_t, entry + 0x64u);
    const uint32_t sum = offset + 0x008943a0u;
    GP4_FIELD(uint32_t, record, 0x84) = sum;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags(offset, 0x008943a0u, sum);
}
GP4_IMPL(0x00424371, FUN_00424371, "regs(eax:ptr[0xe8]:bytes) -> (cf, pf, af, zf, sf, of)")

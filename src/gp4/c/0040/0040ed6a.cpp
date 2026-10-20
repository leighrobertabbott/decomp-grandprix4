#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "cmp left, right" on a byte
static inline uint32_t cmpByteFlags(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) << 4);
}

// flags of "sub left, right" on a dword
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x0040ed6a
// When byte 0x007c2ea9 is 0x80, stores DAT_0066d814 - DAT_0066d838 - ebx in the dword table at
// 0x007c3208, slot ((byte [esi+0x7c] & 0x3f) - 1). pushal/popal preserve every GPR; the flags are
// those of "cmp byte [0x7c2ea9], 0x80" when it differs, else of "sub ecx, 1" (the slot computation).
void __cdecl FUN_0040ed6a(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode != 0x80u) {
        r->eflags = (r->eflags & ~0x8d5u) | cmpByteFlags(mode, 0x80u);
        return;
    }

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x0066d814) - GP4_GLOBAL(uint32_t, 0x0066d838) - r->ebx;
    const uint32_t kind = GP4_FIELD(uint8_t, record, 0x7c) & 0x3fu;
    GP4_ARRAY(uint32_t, 0x007c3208)[(int32_t)(kind - 1u)] = value;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(kind, 1u);
}
GP4_IMPL(0x0040ed6a, FUN_0040ed6a, "regs(ebx:u32, esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

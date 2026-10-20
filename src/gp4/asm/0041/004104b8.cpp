#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Looks up four dword values in the tables at 0x0062f17c / 0x0062f1a8 / 0x0062f1d4 / 0x0062f200
// with the high and low nibbles of the bytes at +0x266 and +0x267 of the record in esi, and
// stores them at 0x00623be0, 0x00623be8, 0x00623be4 and 0x00623bec. eax leaves holding the last
// value; the flags are those of the final "and eax, 0xf" (CF = OF = AF = 0).
// FUNCTION: GP4 0x004104b8
void __cdecl FUN_004104b8(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t first = GP4_FIELD(uint8_t, record, 0x266);
    const uint32_t second = GP4_FIELD(uint8_t, record, 0x267);

    GP4_GLOBAL(uint32_t, 0x00623be0) = GP4_ARRAY(uint32_t, 0x0062f17c)[first >> 4];
    GP4_GLOBAL(uint32_t, 0x00623be8) = GP4_ARRAY(uint32_t, 0x0062f1a8)[first & 0xfu];
    GP4_GLOBAL(uint32_t, 0x00623be4) = GP4_ARRAY(uint32_t, 0x0062f1d4)[second >> 4];
    const uint32_t value = GP4_ARRAY(uint32_t, 0x0062f200)[second & 0xfu];
    GP4_GLOBAL(uint32_t, 0x00623bec) = value;

    const uint32_t index = second & 0xfu;
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag8(index) | (index == 0u ? 0x40u : 0u);
}
GP4_IMPL(0x004104b8, FUN_004104b8, "regs(esi:ptr[0x270]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

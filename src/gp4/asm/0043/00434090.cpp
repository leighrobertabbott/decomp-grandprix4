#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Writes AL into byte edx of the 0x1c-byte record selected by ((byte [esi+0x7c] & 0x3f) - 1),
// capped at 21, in the table at 0x0062ffac. ecx is preserved; the flags are those of the final
// "add ecx, 0x62ffac" (the imul before it is overwritten).
// FUNCTION: GP4 0x00434090
void __cdecl FUN_00434090(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, body, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }

    const uint32_t offset = static_cast<uint32_t>(index) * 0x1cu;
    const uint32_t base = 0x0062ffacu;
    const uint32_t record = offset + base;
    GP4_FIELD(uint8_t, reinterpret_cast<void*>(static_cast<uintptr_t>(record)), r->edx) = static_cast<uint8_t>(r->eax);

    const uint32_t flags = (record < offset ? 1u : 0u)
        | parityFlag(record)
        | (((offset ^ base ^ record) & 0x10u) != 0u ? 0x10u : 0u)
        | (record == 0u ? 0x40u : 0u)
        | ((record & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(offset ^ base) & (offset ^ record) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434090, FUN_00434090, "regs(esi:ptr[0x84]:bytes, edx:u32[0..27], eax:u32) -> (cf, pf, af, zf, sf, of)")

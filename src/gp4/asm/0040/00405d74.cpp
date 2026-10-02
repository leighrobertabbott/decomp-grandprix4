#include <gp4/gp4.h>

static uint32_t ResultFlags(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((~parity & 1u) << 2) | (value == 0 ? 0x40u : 0u)
        | (value & 0x80000000u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00405d74
void __cdecl FUN_00405d74(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->edi));
    const uint32_t stored = GP4_FIELD(const uint32_t, record, 0x44);
    uint32_t value = stored;
    uint32_t flags = ResultFlags(stored);
    if (stored == 0) {
        const bool lower = r->edi < GP4_GLOBAL(uint32_t, 0x00866f10);
        const uint32_t base = lower ? GP4_GLOBAL(uint32_t, 0x00866f04) : GP4_GLOBAL(uint32_t, 0x00866f00);
        const uint32_t adjustment = lower ? GP4_GLOBAL(uint32_t, 0x00866ef8) : GP4_GLOBAL(uint32_t, 0x00891500);
        const uint32_t displacement = r->edi - base;
        value = displacement + adjustment;
        flags = ResultFlags(value) | 1u | ((displacement ^ adjustment ^ value) & 0x10u)
            | ((~(displacement ^ adjustment) & (displacement ^ value) & 0x80000000u) >> 20);
    }
    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405d74, FUN_00405d74, "regs(edi:ptr[0x4c]:layout=unknown_edi_00405d74) -> (eax,cf,pf,af,zf,sf,of); globals=fuzz:i32")

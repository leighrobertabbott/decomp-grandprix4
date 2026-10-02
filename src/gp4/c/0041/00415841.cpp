#include <gp4/gp4.h>

// FUNCTION: GP4 0x00415841
void __cdecl FUN_00415841(gp4::Regs* r) {
    // ebx is loaded with the base of the 0x6380d8 table and left there
    const uint32_t base = 0x006380d8u;
    uint8_t* table = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(base));

    GP4_GLOBAL(uint32_t, 0x0062fb44) = GP4_GLOBAL(uint32_t, 0x00638ce0);

    int32_t value = GP4_GLOBAL(int32_t, 0x0063804c) >> 8;
    GP4_GLOBAL(int32_t, 0x00638cd8) = value;
    GP4_FIELD(int32_t, table, 0x190) = value;
    GP4_FIELD(int32_t, table, 0x490) = value;
    GP4_FIELD(int32_t, table, 0x790) = value;
    GP4_FIELD(int32_t, table, 0xa90) = value;

    GP4_GLOBAL(int32_t, 0x00638cdc) = GP4_GLOBAL(int32_t, 0x00638050) >> 8;

    const uint32_t source = GP4_GLOBAL(uint32_t, 0x00638054);
    value = static_cast<int32_t>(source) >> 8;
    GP4_GLOBAL(int32_t, 0x00638ce0) = value;
    GP4_FIELD(int32_t, table, 0x194) = value;
    GP4_FIELD(int32_t, table, 0x494) = value;
    GP4_FIELD(int32_t, table, 0x794) = value;
    GP4_FIELD(int32_t, table, 0xa94) = value;

    r->ebx = base;
    r->eax = static_cast<uint32_t>(value);
    // last flag writer is sar eax, 8: CF = bit 7 of the source, OF = 0, AF cleared in the pinned model
    const uint32_t result = static_cast<uint32_t>(value);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = ((source >> 7) & 1u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415841, FUN_00415841, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

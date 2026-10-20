#include <gp4/gp4.h>

// FUNCTION: GP4 0x004157ec
void __cdecl FUN_004157ec(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062fb44) = GP4_GLOBAL(uint32_t, 0x00638ce0);
    GP4_GLOBAL(uint32_t, 0x00623e24) = GP4_GLOBAL(uint32_t, 0x0063804c);
    GP4_GLOBAL(uint32_t, 0x00623e28) = GP4_GLOBAL(uint32_t, 0x00638050);
    GP4_GLOBAL(uint32_t, 0x00623e2c) = GP4_GLOBAL(uint32_t, 0x00638054);

    // the callee leaves ebp and edx; they stay in the registers
    gp4::call_regs(0x00401a33, *r);

    GP4_GLOBAL(int32_t, 0x00638cd8) = GP4_GLOBAL(int32_t, 0x00623e18) >> 8;
    GP4_GLOBAL(int32_t, 0x00638cdc) = GP4_GLOBAL(int32_t, 0x00623e1c) >> 8;
    const int32_t source = GP4_GLOBAL(int32_t, 0x00623e20);
    const int32_t value = source >> 8;
    GP4_GLOBAL(int32_t, 0x00638ce0) = value;

    // the last flag writer is "sar eax, 8": CF = bit 7 of the source, OF = AF = 0,
    // SF/ZF/PF from the result
    const uint32_t result = static_cast<uint32_t>(value);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    const uint32_t flags = ((static_cast<uint32_t>(source) >> 7) & 1u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7);
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004157ec, FUN_004157ec, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

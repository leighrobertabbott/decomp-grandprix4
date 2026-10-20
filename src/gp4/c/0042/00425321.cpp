#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of sub left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

struct Unknown_00425321_Block {
    int32_t UnknownX;
    int32_t UnknownY;
    int32_t UnknownZ;
    uint8_t Unknown0C[0x18 - 0x0c];
    int32_t Unknown18;
    int32_t Unknown1C;
    int32_t Unknown20;
    uint8_t Unknown24[0x40 - 0x24];
    uint8_t* Body40;
};

// Subtracts the block's x/y/z (DAT_00609860 -> +0/+4/+8) from DAT_00607f28/2c/30 into
// DAT_00623e24/28/2c, converts them to doubles at +0xa0/+0xa8/+0xb0 of the body at block+0x40,
// multiplies them with 0x00401e2e (results at +0xb8/+0xc0/+0xc8) and rounds the three results
// into block+0x18/+0x1c/+0x20. Leaves ebx = block, edx = body, eax = the Z difference and the
// flags of that last subtraction.
// FUNCTION: GP4 0x00425321
void __cdecl FUN_00425321(gp4::Regs* r) {
    Unknown_00425321_Block* block =
        reinterpret_cast<Unknown_00425321_Block*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));

    GP4_GLOBAL(uint32_t, 0x00623e24) = GP4_GLOBAL(uint32_t, 0x00607f28) - static_cast<uint32_t>(block->UnknownX);
    GP4_GLOBAL(uint32_t, 0x00623e28) = GP4_GLOBAL(uint32_t, 0x00607f2c) - static_cast<uint32_t>(block->UnknownY);
    const uint32_t lastLeft = GP4_GLOBAL(uint32_t, 0x00607f30);
    const uint32_t lastRight = static_cast<uint32_t>(block->UnknownZ);
    const uint32_t lastDiff = lastLeft - lastRight;
    GP4_GLOBAL(uint32_t, 0x00623e2c) = lastDiff;

    uint8_t* body = block->Body40;
    GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e24));
    GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e28));
    GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e2c));

    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(block));
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401e2e, *r);

    block->Unknown18 = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
    block->Unknown1C = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
    block->Unknown20 = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));

    r->eax = lastDiff;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags(lastLeft, lastRight);
}
GP4_IMPL(0x00425321, FUN_00425321, "regs() -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=view_00609860")

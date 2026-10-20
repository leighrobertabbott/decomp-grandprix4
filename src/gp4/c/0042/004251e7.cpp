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

// pushal ... popal: every register comes back; the flags of the last writer stay.
// With DAT_0063963d != 0 that is "cmp byte [0x63963d], 0" (CF = OF = AF = 0, ZF = 0), otherwise
// the "sub eax, [0x6082a8]" of the Z difference.
// Copies the point behind DAT_0060982c (x/y/z at +0/+4/+8) to the one behind DAT_00609860, or,
// when DAT_0063963d == 0, subtracts DAT_006082a0/a4/a8 first, rotates the result with
// 0x00401e2e (body 0x00622cf8) and stores the rounded vector there.
// FUNCTION: GP4 0x004251e7
void __cdecl FUN_004251e7(gp4::Regs* r) {
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0060982c)));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0063963d);

    uint32_t flags;
    if (mode != 0u) {
        GP4_FIELD(uint32_t, dest, 0x00) = GP4_FIELD(uint32_t, source, 0x00);
        GP4_FIELD(uint32_t, dest, 0x04) = GP4_FIELD(uint32_t, source, 0x04);
        GP4_FIELD(uint32_t, dest, 0x08) = GP4_FIELD(uint32_t, source, 0x08);
        flags = parityFlag(mode) | (mode & 0x80u);
    } else {
        GP4_GLOBAL(uint32_t, 0x00623e30) = GP4_FIELD(uint32_t, source, 0x00) - GP4_GLOBAL(uint32_t, 0x006082a0);
        GP4_GLOBAL(uint32_t, 0x00623e34) = GP4_FIELD(uint32_t, source, 0x04) - GP4_GLOBAL(uint32_t, 0x006082a4);
        const uint32_t lastLeft = GP4_FIELD(uint32_t, source, 0x08);
        const uint32_t lastRight = GP4_GLOBAL(uint32_t, 0x006082a8);
        GP4_GLOBAL(uint32_t, 0x00623e38) = lastLeft - lastRight;
        flags = subFlags(lastLeft, lastRight);

        uint8_t* body = GP4_ARRAY(uint8_t, 0x00622cf8);
        GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e30));
        GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e34));
        GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e38));

        gp4::Regs call = *r;
        call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
        gp4::call_regs(0x00401e2e, call);

        GP4_FIELD(int32_t, dest, 0x00) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
        GP4_FIELD(int32_t, dest, 0x04) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
        GP4_FIELD(int32_t, dest, 0x08) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004251e7, FUN_004251e7, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=view_00609860")

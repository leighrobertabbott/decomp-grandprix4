#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of sar value, 5 (pinned oracle model: CF = last bit shifted out, AF = OF = 0)
static inline uint32_t sar5Flags(int32_t value) {
    const uint32_t result = static_cast<uint32_t>(value >> 5);
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((static_cast<uint32_t>(value) >> 4) & 1u);
}

// pushal ... popal: every register comes back; the flags of the last "sar eax, 5" stay (the one
// of the dword at +0x20 of the source).
// Copies the point behind DAT_0060982c (+0x18/+0x1c/+0x20, shifted right by 5) to +0xc/+0x10/+0x14
// of the block behind DAT_00609860, or, when DAT_0063963d == 0, rotates it first with 0x00401e2e
// (body 0x00622cf8) and stores the rounded vector there.
// FUNCTION: GP4 0x00425282
void __cdecl FUN_00425282(gp4::Regs* r) {
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0060982c)));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0063963d);

    const int32_t x = GP4_FIELD(int32_t, source, 0x18);
    const int32_t y = GP4_FIELD(int32_t, source, 0x1c);
    const int32_t z = GP4_FIELD(int32_t, source, 0x20);

    if (mode != 0u) {
        GP4_FIELD(int32_t, dest, 0x0c) = x >> 5;
        GP4_FIELD(int32_t, dest, 0x10) = y >> 5;
        GP4_FIELD(int32_t, dest, 0x14) = z >> 5;
    } else {
        GP4_GLOBAL(int32_t, 0x00623e30) = x >> 5;
        GP4_GLOBAL(int32_t, 0x00623e34) = y >> 5;
        GP4_GLOBAL(int32_t, 0x00623e38) = z >> 5;

        uint8_t* body = GP4_ARRAY(uint8_t, 0x00622cf8);
        GP4_FIELD(double, body, 0xa0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e30));
        GP4_FIELD(double, body, 0xa8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e34));
        GP4_FIELD(double, body, 0xb0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e38));

        gp4::Regs call = *r;
        call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
        gp4::call_regs(0x00401e2e, call);

        GP4_FIELD(int32_t, dest, 0x0c) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
        GP4_FIELD(int32_t, dest, 0x10) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
        GP4_FIELD(int32_t, dest, 0x14) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));
    }
    r->eflags = (r->eflags & ~0x8d5u) | sar5Flags(z);
}
GP4_IMPL(0x00425282, FUN_00425282, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=view_00609860")

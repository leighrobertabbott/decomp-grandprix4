#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parity8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a 32-bit `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parity8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul then shrd eax, edx, n: the low dword of the signed 64-bit product shifted right by n.
static inline uint32_t mulShr(uint32_t a, uint32_t b, int n) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
}

// The orientation body at 0x00622dc8 (edx in the original), addressed through its base.
static __declspec(noinline) uint8_t* orientationBody() {
    return GP4_ARRAY(uint8_t, 0x00622dc8);
}

// pushal ... popal routine. esi = a block whose dword +0x74 points to its source object. When bit 7
// of the object's byte +0x7a is set nothing happens and CF = 1 (flags of that test otherwise).
// Otherwise 0x004045d9 latches the object's position, the angles (object dwords +0x10 / +0xc and
// word [0x00934e88] << 16) go to block +0xc0..+0xc8 and to the body 0x00622dc8, whose rotation
// (0x00401d5c / 0x00401d80 / 0x00401da2) transforms the block's offset +0xa8..+0xb0 (0x00401f02);
// the rounded result plus the object's position >> 5 becomes block +0x18 / +0x1c / +0x20 (with
// +0x24..+0x2c cleared). Then 0x004049bf / 0x00402fcd give a second vector whose components >> 13,
// scaled by block +0x70 / +0x6c (and offset by +0x34 for the middle one) and twice by the clamped
// depth max(+0x44 - +0x50, 0), are added to those three dwords. CF = 0, the other flags are those of
// the final add.
// FUNCTION: GP4 0x00443e69
void __cdecl FUN_00443e69(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, block, 0x74)));

    if ((GP4_FIELD(uint8_t, object, 0x7a) & 0x80u) != 0u) {
        // test byte, 0x80 (nonzero: SF), then stc
        r->eflags = (r->eflags & ~0x8d5u) | 0x80u | 0x01u;
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = reinterpret_cast<uint32_t>(object);
    c.edi = GP4_FIELD(uint32_t, object, 0x1c);
    gp4::call_regs(0x004045d9, c);
    c.edi = reinterpret_cast<uint32_t>(block);

    uint8_t* body = orientationBody();
    c.edx = reinterpret_cast<uint32_t>(body);
    const uint32_t first = GP4_FIELD(uint32_t, object, 0x10);
    GP4_FIELD(uint32_t, block, 0xc0) = first;
    GP4_FIELD(uint32_t, body, 0) = first;
    const uint32_t second = GP4_FIELD(uint32_t, object, 0xc);
    GP4_FIELD(uint32_t, block, 0xc4) = second;
    GP4_FIELD(uint32_t, body, 4) = second;
    const uint32_t third = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00934e88)) << 16;
    GP4_FIELD(uint32_t, block, 0xc8) = third;
    GP4_FIELD(uint32_t, body, 8) = third;
    c.eax = third;
    gp4::call_regs(0x00401d5c, c);
    gp4::call_regs(0x00401d80, c);
    gp4::call_regs(0x00401da2, c);

    GP4_GLOBAL(uint32_t, 0x00623e18) = GP4_FIELD(uint32_t, block, 0xa8);
    GP4_GLOBAL(uint32_t, 0x00623e1c) = GP4_FIELD(uint32_t, block, 0xac);
    GP4_GLOBAL(uint32_t, 0x00623e20) = GP4_FIELD(uint32_t, block, 0xb0);
    c.eax = GP4_FIELD(uint32_t, block, 0xb0);
    GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e18));
    GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e1c));
    GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e20));
    gp4::call_regs(0x00401f02, c);

    uint8_t* transformed = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edx));
    GP4_GLOBAL(int32_t, 0x00623e30) = fistp32(GP4_FIELD(double, transformed, 0xa0));
    GP4_GLOBAL(int32_t, 0x00623e34) = fistp32(GP4_FIELD(double, transformed, 0xa8));
    GP4_GLOBAL(int32_t, 0x00623e38) = fistp32(GP4_FIELD(double, transformed, 0xb0));

    GP4_FIELD(uint32_t, block, 0x18) = static_cast<uint32_t>(GP4_FIELD(int32_t, object, 0) >> 5) + GP4_GLOBAL(uint32_t, 0x00623e30);
    GP4_FIELD(uint32_t, block, 0x24) = 0u;
    GP4_FIELD(uint32_t, block, 0x20) = static_cast<uint32_t>(GP4_FIELD(int32_t, object, 4) >> 5) + GP4_GLOBAL(uint32_t, 0x00623e38);
    GP4_FIELD(uint32_t, block, 0x2c) = 0u;
    GP4_FIELD(uint32_t, block, 0x1c) = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00934df8) >> 5) + GP4_GLOBAL(uint32_t, 0x00623e34);
    GP4_FIELD(uint32_t, block, 0x28) = 0u;

    c.edi = GP4_FIELD(uint32_t, object, 0x1c);
    gp4::call_regs(0x004049bf, c);
    gp4::call_regs(0x00402fcd, c);
    GP4_GLOBAL(uint32_t, 0x00623e34) = c.eax;

    const uint32_t x = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e30) >> 13);
    const uint32_t z = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e38) >> 13);
    const uint32_t y = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e34) >> 13);
    uint32_t depth = GP4_FIELD(uint32_t, block, 0x44) - GP4_FIELD(uint32_t, block, 0x50);
    if (static_cast<int32_t>(depth) < 0) {
        depth = 0u;
    }

    uint32_t term = 0u - mulShr(x, GP4_FIELD(uint32_t, block, 0x70), 14);
    term = mulShr(mulShr(term, depth, 16), depth, 16);
    GP4_FIELD(uint32_t, block, 0x18) += term;

    term = 0u - mulShr(z, GP4_FIELD(uint32_t, block, 0x70), 14);
    term = mulShr(mulShr(term, depth, 16), depth, 16);
    GP4_FIELD(uint32_t, block, 0x20) += term;

    term = GP4_FIELD(uint32_t, block, 0x34) - mulShr(y, GP4_FIELD(uint32_t, block, 0x6c), 14);
    term = mulShr(mulShr(term, depth, 16), depth, 16);
    const uint32_t before = GP4_FIELD(uint32_t, block, 0x1c);
    GP4_FIELD(uint32_t, block, 0x1c) = before + term;

    // popal, clc: the flags of the final add with CF cleared
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags(before, term) & ~0x01u);
}
GP4_IMPL(0x00443e69, FUN_00443e69, "regs(esi:ptr[0xd0]:layout=rec_00443e69) -> (cf, pf, af, zf, sf, of)")

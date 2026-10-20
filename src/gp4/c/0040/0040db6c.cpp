#include <gp4/gp4.h>

// Flags of the 16-bit `cmp left, right` (also of `sub left, right`).
static inline uint32_t compareFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Subtracts DAT_0066d88a from a word and wraps it into 0 .. DAT_00868558 (one add, one subtract).
// Returns the wrapped word; `flags` receives the flags of the last compare / subtract.
static inline uint16_t wrapWord(uint16_t value, uint32_t* flags) {
    const uint16_t period = GP4_GLOBAL(uint16_t, 0x00868558);
    uint16_t word = static_cast<uint16_t>(value - GP4_GLOBAL(uint16_t, 0x0066d88a));
    if ((word & 0x8000u) != 0) {
        word = static_cast<uint16_t>(word + period);
    }
    *flags = compareFlags16(word, period);
    if (static_cast<int16_t>(word) >= static_cast<int16_t>(period)) {
        word = static_cast<uint16_t>(word - period);
    }
    return word;
}

// Fills the four lane words of the car at esi: +0x148/+0x14a (left lane position, fraction) and
// +0x14c/+0x14e (right lane). With DAT_00604430 clear they are the car's words +0x68/+0x66. Otherwise
// they come from the lane table of the car's current segment (+0x1c): behind the pit end
// [0x00891500] from 0x00405985 (position folded by the scale) and 0x0040daeb, in front of it from the
// table entry and 0x0040da62. Both positions are then moved by DAT_0066d88a into 0 .. DAT_00868558.
// pushal/popal keep every register; the flags are those of the last compare (DAT_00604430 compared
// with zero when the lane words are plain copies).
// FUNCTION: GP4 0x0040db6c
void __cdecl FUN_0040db6c(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    if (GP4_GLOBAL(uint8_t, 0x00604430) == 0) {
        flags = 0x44u;
        GP4_FIELD(uint16_t, car, 0x148) = GP4_FIELD(uint16_t, car, 0x68);
        GP4_FIELD(uint16_t, car, 0x14c) = GP4_FIELD(uint16_t, car, 0x68);
        GP4_FIELD(uint16_t, car, 0x14a) = GP4_FIELD(uint16_t, car, 0x66);
        GP4_FIELD(uint16_t, car, 0x14e) = GP4_FIELD(uint16_t, car, 0x66);
    } else {
        uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, car, 0x1c)));
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment));

        if (static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment)) >= GP4_GLOBAL(uint32_t, 0x00891500)) {
            c.eax = (c.eax & 0xffff0000u) | (GP4_FIELD(uint16_t, segment, 0x1a) & 0x7ffu);
            c.ecx = (c.ecx & 0xffff0000u) |
                    static_cast<uint16_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, car, 0x66)) >> 8);
            gp4::call_regs(0x00405985, c);

            uint16_t position = static_cast<uint16_t>(c.eax);
            if (static_cast<int16_t>(position) >= GP4_GLOBAL(int16_t, 0x00868558)) {
                position = static_cast<uint16_t>(position - GP4_GLOBAL(uint16_t, 0x00868558));
            }
            GP4_FIELD(uint16_t, car, 0x14c) = position;
            GP4_FIELD(uint16_t, car, 0x14e) = static_cast<uint16_t>(static_cast<uint16_t>(c.ecx) << 8);
            gp4::call_regs(0x0040daeb, c);
            GP4_FIELD(uint16_t, car, 0x148) = static_cast<uint16_t>(c.edx);
            GP4_FIELD(uint16_t, car, 0x14a) = static_cast<uint16_t>(c.eax);
        } else {
            const uint32_t entry = 0x00604435u + static_cast<uint32_t>(GP4_FIELD(uint16_t, segment, 0x7a)) * 8u;
            c.ecx = entry;
            GP4_FIELD(uint16_t, car, 0x148) = GP4_FIELD(uint16_t, entry, 0);
            c.eax = (c.eax & 0xffff0000u) | GP4_FIELD(uint16_t, entry, 0);
            GP4_FIELD(uint16_t, car, 0x14a) = GP4_FIELD(uint16_t, car, 0x66);
            c.eax = (c.eax & 0xffff0000u) | GP4_FIELD(uint16_t, car, 0x66);
            gp4::call_regs(0x0040da62, c);
            GP4_FIELD(uint16_t, car, 0x14c) = static_cast<uint16_t>(c.edx);
            GP4_FIELD(uint16_t, car, 0x14e) = static_cast<uint16_t>(c.eax);
        }

        uint32_t unused;
        GP4_FIELD(uint16_t, car, 0x148) = wrapWord(GP4_FIELD(uint16_t, car, 0x148), &unused);
        GP4_FIELD(uint16_t, car, 0x14c) = wrapWord(GP4_FIELD(uint16_t, car, 0x14c), &flags);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040db6c, FUN_0040db6c, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+lanes_0040da62+pend_0040daeb+state_004094e3")

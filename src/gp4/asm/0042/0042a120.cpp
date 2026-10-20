#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint8_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 16-bit logic result (CF = OF = AF = 0).
static inline uint32_t logicFlags16(uint16_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags (without CF) of "neg" on a nonzero 16-bit value.
static inline uint32_t negFlags16(uint16_t value) {
    const uint16_t result = static_cast<uint16_t>(0u - value);
    return parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (value == 0x8000u ? 0x800u : 0u);
}

struct Unknown_0042a120_Record {
    uint8_t Unknown00[0xea];
    uint16_t Unknown0EA;
    uint8_t Unknown0EC[0xfe - 0xec];
    uint8_t Unknown0FE;
    uint8_t Unknown0FF[0x114 - 0xff];
    uint32_t Unknown114;
};

// FUNCTION: GP4 0x0042a120
// esi = a car-like record whose word +0xea is a signed direction (0 = none), ebx = a second record. The result goes
// to DAT_0060414c (byte) and DAT_0060414e (word, +-0x10 / +-8 / 0) and CF comes back clear. With a nonzero word
// and ebx == dword +0x114 of esi nothing is stored and CF comes back set. The low words of edx and ecx change
// (dx = the loaded word with dl cleared / the selector byte, cx = the stored word); the flags are those of the
// last flag writer before the final "clc".
void __cdecl FUN_0042a120(gp4::Regs* r) {
    const Unknown_0042a120_Record* self = reinterpret_cast<const Unknown_0042a120_Record*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042a120_Record* other = reinterpret_cast<const Unknown_0042a120_Record*>(static_cast<uintptr_t>(r->ebx));

    const uint16_t word = self->Unknown0EA;
    uint16_t dx;
    uint8_t selector = 0;
    uint16_t wordOut;
    uint32_t flags;

    if (word != 0u) {
        if (r->ebx == self->Unknown114) {
            // cmp ebx, [esi+0x114] equal (CF = AF = SF = OF = 0, ZF = PF = 1), then stc
            r->edx = (r->edx & 0xffff0000u) | word;
            r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
            return;
        }
        const uint16_t mixed = static_cast<uint16_t>(
            ((static_cast<uint16_t>(GP4_GLOBAL(uint8_t, 0x00604154)) << 8) | (r->ecx & 0xffu)) ^ word);
        if (static_cast<int16_t>(mixed) < 0) {
            wordOut = 0;
            flags = logicFlags16(mixed);
        } else if (static_cast<int16_t>(word) < 0) {
            wordOut = 0xfff0;
            flags = negFlags16(0x10);
        } else {
            wordOut = 0x10;
            flags = logicFlags16(word);
        }
        dx = static_cast<uint16_t>(word & 0xff00u);
    } else {
        const uint8_t otherBits = other->Unknown0FE;
        const uint8_t selfBits = self->Unknown0FE;
        if ((otherBits & 0x40u) != 0u) {
            selector = (otherBits & 0x10u) != 0u ? 0x80 : 0x40;
        } else if ((selfBits & 0x40u) != 0u) {
            selector = (selfBits & 0x10u) != 0u ? 0x80 : 0x40;
        }

        const uint8_t marker = GP4_GLOBAL(uint8_t, 0x00604154);
        const bool selectorRight = selector != 0u && static_cast<int8_t>(marker ^ selector) >= 0;
        const bool otherActive = (otherBits & 0x40u) != 0u;

        if (selector != 0u && otherActive && !selectorRight) {
            // 0x42a1b6: +-8, selector ^ 0xc0 (flags of that xor)
            wordOut = static_cast<int8_t>(marker) < 0 ? 0xfff8 : 8;
            selector = static_cast<uint8_t>(selector ^ 0xc0u);
            flags = logicFlags8(selector);
        } else if (selector != 0u && otherActive) {
            // 0x42a1cb
            selector = static_cast<uint8_t>(selector ^ 0xc0u);
            wordOut = 0;
            flags = logicFlags8(selector);
        } else if (selectorRight) {
            // 0x42a1d4: the zero result of "test byte [ebx+0xfe], 0x40"
            selector = 0;
            wordOut = 0;
            flags = 0x44u;
        } else {
            // 0x42a1dc: +-8 by the sign of the marker byte
            selector = 0;
            if (static_cast<int8_t>(marker) < 0) {
                wordOut = 0xfff8;
                flags = negFlags16(8);
            } else {
                wordOut = 8;
                flags = logicFlags8(marker);
            }
        }
        dx = selector;
    }

    GP4_GLOBAL(uint8_t, 0x0060414c) = static_cast<uint8_t>(dx);
    GP4_GLOBAL(uint16_t, 0x0060414e) = wordOut;

    r->edx = (r->edx & 0xffff0000u) | dx;
    r->ecx = (r->ecx & 0xffff0000u) | wordOut;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u);
}
GP4_IMPL(0x0042a120, FUN_0042a120, "regs(ebx:rec[0x66fef0,0x41c,0..21], ecx:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+link_0042a120")

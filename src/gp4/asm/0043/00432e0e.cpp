#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic operation (`or` / `xor` on a byte): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_00432e0e_Body {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x66 - 0x10];
    int16_t Unknown66;
    uint8_t Unknown68[0x90 - 0x68];
    int16_t Unknown90;
    uint8_t Unknown92[0x9a - 0x92];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xcb - 0x9c];
    uint8_t UnknownCB;
    uint8_t UnknownCC[0x11c - 0xcc];
    int16_t Unknown11C;
    int16_t Unknown11E;
    uint8_t Unknown120[0x232 - 0x120];
    uint8_t Unknown232;
};

// FUNCTION: GP4 0x00432e0e
// esi = car record, edi = word pointer. Builds ebp (signed word) from the cosine table
// 0x007c5c5c at index (0x4000 - |+0x0e - [edi]|) and the words +0x90 and +0x11c / +0x11e (picked
// by the sign of byte 0x00868582), then compares it with the thresholds 0x00601adc / ae0 / ae4
// depending on the slot index from 0x00432c21 (kinds 0, 1, 2; index >= 3 tests the sign of
// (+0x90 >> 8) ^ byte 0x00868582). CF set (stc) = rejected; clear (clc) = accepted, in which case
// bit 1 of +0xcb and +0x232 may be set. The other flags are those of the last flag writer.
void __cdecl FUN_00432e0e(gp4::Regs* r) {
    Unknown_00432e0e_Body* self = reinterpret_cast<Unknown_00432e0e_Body*>(static_cast<uintptr_t>(r->esi));
    const int16_t* other = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(r->edi));

    uint16_t ax = static_cast<uint16_t>(self->Unknown0E - *other);
    if ((ax & 0x8000u) != 0u) {
        ax = static_cast<uint16_t>(0u - ax);
    }
    ax = static_cast<uint16_t>(0u - ax);
    ax = static_cast<uint16_t>(ax + 0x4000u);
    if ((ax & 0x8000u) != 0u) {
        ax = static_cast<uint16_t>(0u - ax);
    }
    ax = static_cast<uint16_t>(ax >> 2);
    const uint32_t offset = static_cast<uint32_t>(ax) & 0xfffeu;
    const int32_t cosine = GP4_FIELD(int16_t, GP4_ARRAY(uint8_t, 0x007c5c5c), offset);
    ax = static_cast<uint16_t>((cosine >> 6) + 0xbd);
    ax = static_cast<uint16_t>(0u - ax);

    uint16_t dx;
    if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
        dx = static_cast<uint16_t>(self->Unknown11C + 0x150);
        ax = static_cast<uint16_t>(ax - self->Unknown90);
        ax = static_cast<uint16_t>(ax - dx);
    } else {
        dx = static_cast<uint16_t>(self->Unknown11E + 0x150);
        ax = static_cast<uint16_t>(ax + self->Unknown90);
        ax = static_cast<uint16_t>(ax - dx);
    }
    const int32_t ebp = static_cast<int32_t>(static_cast<int16_t>(ax));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00432c21, call);
    const int32_t index = static_cast<int32_t>(call.eax);
    uint32_t eax = call.eax;

    uint32_t flags = cmpFlags32(static_cast<uint32_t>(index), 0u);
    bool carry = false;
    if (index < 0) {
        carry = true;
    } else if (index == 0) {
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x00601ae4);
        flags = cmpFlags32(static_cast<uint32_t>(ebp), static_cast<uint32_t>(threshold));
        if (ebp < threshold) {
            carry = true;
        } else {
            eax = static_cast<uint32_t>(static_cast<int32_t>(self->Unknown66));
            const int32_t limit = GP4_GLOBAL(int32_t, 0x00627884);
            flags = cmpFlags32(eax, static_cast<uint32_t>(limit));
            if (static_cast<int32_t>(eax) >= limit) {
                carry = true;
            } else {
                const uint16_t angle = static_cast<uint16_t>(self->Unknown9A);
                eax = (eax & 0xffff0000u) | angle;
                flags = cmpFlags16(angle, 0x200u);
                if (static_cast<int16_t>(angle) >= 0x200) {
                    self->UnknownCB |= 2u;
                    self->Unknown232 |= 2u;
                    flags = logicFlags8(self->Unknown232);
                }
            }
        }
    } else if (index == 1) {
        flags = cmpFlags32(static_cast<uint32_t>(index), 1u);
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x00601ae0);
        flags = cmpFlags32(static_cast<uint32_t>(ebp), static_cast<uint32_t>(threshold));
        if (ebp < threshold) {
            carry = true;
        } else {
            const uint16_t angle = static_cast<uint16_t>(self->Unknown9A);
            eax = (eax & 0xffff0000u) | angle;
            flags = cmpFlags16(angle, 0x200u);
            if (static_cast<int16_t>(angle) >= 0x200) {
                self->UnknownCB |= 2u;
                self->Unknown232 |= 2u;
                flags = logicFlags8(self->Unknown232);
            }
        }
    } else if (index == 2) {
        flags = cmpFlags32(static_cast<uint32_t>(index), 2u);
        const int32_t threshold = GP4_GLOBAL(int32_t, 0x00601adc);
        flags = cmpFlags32(static_cast<uint32_t>(ebp), static_cast<uint32_t>(threshold));
        carry = ebp < threshold;
    } else {
        flags = cmpFlags32(static_cast<uint32_t>(index), 2u);
        uint16_t shifted = static_cast<uint16_t>(static_cast<int16_t>(self->Unknown90) >> 8);
        shifted = static_cast<uint16_t>((shifted & 0xff00u) | ((shifted ^ GP4_GLOBAL(uint8_t, 0x00868582)) & 0xffu));
        eax = (eax & 0xffff0000u) | shifted;
        flags = logicFlags8(shifted);
        carry = (shifted & 0x80u) != 0u;
    }

    r->eax = eax;
    r->edx = (r->edx & 0xffff0000u) | dx;
    r->ebp = static_cast<uint32_t>(ebp);
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | (carry ? 1u : 0u);
}
GP4_IMPL(0x00432e0e, FUN_00432e0e, "regs(edx:u32, edi:ptr[0x40]:bytes, esi:ptr[0x240]:layout=rec_00432f18) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00432f18")

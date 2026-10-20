#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
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
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

// Fields of the car records at 0x0066fef0 this routine reads / writes.
struct Unknown_0042fe2f_Car {
    uint8_t Unknown00[0x68];
    int16_t Unknown68;
    uint8_t Unknown6A[0x7b - 0x6a];
    uint8_t Flags7B;
    uint8_t Unknown7C[0x7d - 0x7c];
    uint8_t Index7D;
    uint8_t Unknown7E[0x9a - 0x7e];
    int16_t Unknown9A;
    uint8_t Flags9C;
    uint8_t Unknown9D[0xe2 - 0x9d];
    int16_t UnknownE2;
    uint8_t UnknownE4[0xea - 0xe4];
    uint16_t UnknownEA;
    uint8_t UnknownEC[0x26b - 0xec];
    uint8_t Flags26B;
    uint8_t Unknown26C[0x273 - 0x26c];
    uint8_t Flags273;
};

// FUNCTION: GP4 0x0042fe2f
// Looks through the car-order list (words at 0x0066fe3c, starting 3 slots before the car's own
// index +0x7d, at most 10 entries, a 0xffff word ends it) for a car ahead on the track that
// blocks record esi: another car in state 0x18, or 0x20 without bit 3 of +0x26b (and, when
// [0x7c2ea9] is 0x80, without bit 6 of +0x7b), whose lap-position word +0x68 lies within the
// window [-1, [0x603afe]) of this car's and whose offset (+0xe2, or the 0x40dc8a variant when
// either car has bit 1 of +0x9c) is below [0x603b26] and, negated, below a fraction (1/4, or 1/8
// when [0x7c2ea9] is 0x80) of its speed word +0x9a (>= 0x10). A hit sets bit 0 of +0x273 and
// returns CF = 0, otherwise CF = 1. ebx is left at the last car examined; eax and edx carry the
// last words computed; ecx and edi are restored.
void __cdecl FUN_0042fe2f(gp4::Regs* r) {
    Unknown_0042fe2f_Car* self = reinterpret_cast<Unknown_0042fe2f_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebx = r->ebx;
    uint32_t exitFlags;

    int32_t offset = static_cast<int32_t>(self->Index7D) - 8;
    if (offset < -2) {
        offset = -2;
    }

    uint32_t remaining = 10;
    for (;;) {
        offset += 2;
        ebx = GP4_FIELD(uint16_t, 0x0066fe3c, offset);
        if (static_cast<uint16_t>(ebx) == 0xffffu) {
            exitFlags = cmpFlags16(ebx, 0xffffu);
            break;
        }
        ebx += 0x0066fef0u;

        const Unknown_0042fe2f_Car* other = reinterpret_cast<const Unknown_0042fe2f_Car*>(static_cast<uintptr_t>(ebx));
        bool candidate = ebx != r->esi;
        if (candidate && other->UnknownEA != 0x18u) {
            candidate = other->UnknownEA == 0x20u && (other->Flags26B & 8u) == 0u &&
                (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u || (other->Flags7B & 0x40u) == 0u);
        }

        if (candidate) {
            eax = (eax & 0xffff0000u) |
                static_cast<uint16_t>(static_cast<uint16_t>(self->Unknown68) - static_cast<uint16_t>(other->Unknown68));
            if (static_cast<int16_t>(eax) >= GP4_GLOBAL(int16_t, 0x00603afe)) {
                exitFlags = cmpFlags16(eax, static_cast<uint16_t>(GP4_GLOBAL(int16_t, 0x00603afe)));
                break;
            }
            if (static_cast<int16_t>(eax) >= -1) {
                if ((self->Flags9C & 2u) != 0u || (other->Flags9C & 2u) != 0u) {
                    gp4::Regs call = *r;
                    call.st_in = 0;
                    call.st_out = 0;
                    call.eax = eax;
                    call.ebx = ebx;
                    call.edx = edx;
                    gp4::call_regs(0x0040dc8a, call);
                    eax = call.eax;
                } else {
                    eax = (eax & 0xffff0000u) |
                        static_cast<uint16_t>(static_cast<uint16_t>(other->UnknownE2) - static_cast<uint16_t>(self->UnknownE2));
                }
                if (static_cast<int16_t>(eax) < GP4_GLOBAL(int16_t, 0x00603b26)) {
                    eax = (eax & 0xffff0000u) | static_cast<uint16_t>(0u - static_cast<uint16_t>(eax));
                    if (other->Unknown9A >= 0x10) {
                        const uint32_t shift = (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) ? 3u : 2u;
                        const int16_t limit = static_cast<int16_t>(other->Unknown9A >> shift);
                        edx = (edx & 0xffff0000u) | static_cast<uint16_t>(limit);
                        if (static_cast<int16_t>(eax) < limit) {
                            self->Flags273 = static_cast<uint8_t>(self->Flags273 | 1u);
                            r->eax = eax;
                            r->ebx = ebx;
                            r->edx = edx;
                            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(self->Flags273);
                            return;
                        }
                    }
                }
            }
        }

        remaining -= 1u;
        if (remaining == 0u) {
            exitFlags = 0x44u;  // dec ecx reaching zero: ZF, PF
            break;
        }
    }

    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (exitFlags & 0x8d4u) | 1u;
}
GP4_IMPL(0x0042fe2f, FUN_0042fe2f, "regs(eax:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); cases=500; globals=fuzz:i32+layout=list_0066fe3c+cars_0066fef0+state_004059ef+carx_0042fe2f")

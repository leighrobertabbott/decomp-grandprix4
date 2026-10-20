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
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// The two car bodies (esi = this car, ebx = the other one).
struct Unknown_0042f05a_Body {
    uint8_t Unknown00[0x9c];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xcb - 0x9d];
    uint8_t FlagsCB;
    uint8_t UnknownCC[2];
    uint8_t FlagsCE;
    uint8_t UnknownCF[0xd4 - 0xcf];
    uint16_t UnknownD4;
    uint8_t UnknownD6[0xe2 - 0xd6];
    uint16_t UnknownE2;
};

// esi / ebx = the two bodies. Sets bit 3 of [esi+0xcb], takes the word difference of their +0xe2 (or
// the 0x0040dc8a value when either has bit 1 of +0x9c set) into word 0x00603e70 and copies the cap word
// 0x00623e04 to 0x00623e08. Then 0x0042a792 gives the factor stored at word [esi+0xd4]; the factor is
// replaced by 0x4000 when the value at 0x00603e94 is below the limit 0x00603974 (bit 7 of +0xce set) or
// 0x00603968 (clear); otherwise, with bit 7 clear and the difference below the word 0x00603b52, it is
// scaled by 0x0042efee (imul / shrd 14); with the difference at or above it, and 0x00603e94 below
// 0x00603970 (or bit 7 set and 0x00603e94 at or above 0x00603974), 0x0042a533 supplies the new factor.
// eax, ecx and edx are what the last helper (or the final scale) leaves; the flags are those of the
// last compare / helper.
// FUNCTION: GP4 0x0042f05a
void __cdecl FUN_0042f05a(gp4::Regs* r) {
    Unknown_0042f05a_Body* self = reinterpret_cast<Unknown_0042f05a_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042f05a_Body* other = reinterpret_cast<const Unknown_0042f05a_Body*>(static_cast<uintptr_t>(r->ebx));

    self->FlagsCB = static_cast<uint8_t>(self->FlagsCB | 0x08u);

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint16_t difference;
    if ((self->Flags9C & 2u) == 0u && (other->Flags9C & 2u) == 0u) {
        difference = static_cast<uint16_t>(other->UnknownE2 - self->UnknownE2);
        s.eax = (s.eax & 0xffff0000u) | difference;
    } else {
        gp4::call_regs(0x0040dc8a, s);
        difference = static_cast<uint16_t>(s.eax);
    }
    GP4_GLOBAL(uint16_t, 0x00603e70) = difference;
    GP4_GLOBAL(uint16_t, 0x00623e08) = GP4_GLOBAL(uint16_t, 0x00623e04);

    gp4::call_regs(0x0042a792, s);
    self->UnknownD4 = static_cast<uint16_t>(s.ecx);

    uint32_t flags = s.eflags;
    bool useRate = false;   // 0x0042f109: factor from 0x0042a533
    bool useCap = false;    // 0x0042f117: factor = 0x4000
    if ((self->FlagsCE & 0x80u) != 0u) {
        s.edx = GP4_GLOBAL(uint32_t, 0x00603e94);
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00603974);
        flags = cmpFlags32(s.edx, limit);
        if (static_cast<int32_t>(s.edx) < static_cast<int32_t>(limit)) {
            useCap = true;
        } else {
            useRate = true;
        }
    } else {
        s.edx = GP4_GLOBAL(uint32_t, 0x00603e94);
        const uint32_t lower = GP4_GLOBAL(uint32_t, 0x00603968);
        flags = cmpFlags32(s.edx, lower);
        if (static_cast<int32_t>(s.edx) < static_cast<int32_t>(lower)) {
            useCap = true;
        } else {
            const uint32_t current = GP4_GLOBAL(uint16_t, 0x00603e70);
            s.eax = (s.eax & 0xffff0000u) | current;
            const uint32_t bound = GP4_GLOBAL(uint16_t, 0x00603b52);
            flags = cmpFlags16(current, bound);
            if (static_cast<int16_t>(current) < static_cast<int16_t>(bound)) {
                gp4::call_regs(0x0042efee, s);
                const int32_t factor = static_cast<int16_t>(self->UnknownD4);
                const int64_t product = static_cast<int64_t>(static_cast<int32_t>(s.eax)) * static_cast<int64_t>(factor);
                const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
                const uint32_t low = static_cast<uint32_t>(product);
                s.edx = high;
                // shrd eax, edx, 14: CF = last bit shifted out, AF = 0, OF = bit31 of (value >> 13) ^ result.
                const uint64_t wide = (static_cast<uint64_t>(high) << 32) | low;
                const uint32_t previous = static_cast<uint32_t>(wide >> 13);
                const uint32_t result = static_cast<uint32_t>(wide >> 14);
                flags = ((low >> 13) & 1u)
                    | parityFlag8(result)
                    | (result == 0u ? 0x40u : 0u)
                    | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
                    | ((((previous ^ result) & 0x80000000u) != 0u) ? 0x800u : 0u);
                s.eax = result;
                self->UnknownD4 = static_cast<uint16_t>(result);
            } else {
                s.edx = GP4_GLOBAL(uint32_t, 0x00603e94);
                const uint32_t upper = GP4_GLOBAL(uint32_t, 0x00603970);
                flags = cmpFlags32(s.edx, upper);
                if (static_cast<int32_t>(s.edx) < static_cast<int32_t>(upper)) {
                    useRate = true;
                }
            }
        }
    }

    if (useRate) {
        gp4::call_regs(0x0042a533, s);
        flags = s.eflags;
        self->UnknownD4 = static_cast<uint16_t>(s.ecx);
    } else if (useCap) {
        self->UnknownD4 = 0x4000u;
    }

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x0042f05a, FUN_0042f05a, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x225]:layout=rec_0042a9cf+rec_0042efee, ebx:ptr[0x225]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00426091+state_0042efee+gap_0042a533+gain_004255d4")

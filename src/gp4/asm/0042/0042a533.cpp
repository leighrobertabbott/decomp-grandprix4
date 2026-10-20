#include <gp4/gp4.h>

struct Unknown_0042a533_Body {
    uint8_t Unknown00[0x9a];
    uint16_t Unknown9A;
    uint8_t Unknown9C;
    uint8_t Unknown9D[0xcc - 0x9d];
    uint8_t UnknownCC;
    uint8_t UnknownCD[0xe2 - 0xcd];
    uint16_t UnknownE2;
};

// FUNCTION: GP4 0x0042a533
// esi / ebx = the two bodies. ax = difference of their words +0xe2 (or the 0x0040dc8a value when
// either has bit 1 of +0x9c set), minus word 0x00603b98 when bit 3 of [esi+0xcc] is set, minus word
// 0x00603b50. The word delta = [ebx+0x9a] - [esi+0x9a] (plus (ax * [0x00623df6]) >> 10 when ax <= 0)
// is stored to 0x00934dec. ax <= 0: n = delta^2 >> 4; n >= 0x1000 gives the cap word 0x00623e04,
// else cx = hi16(n * word [0x00603854]). ax > 0: ax -= word 0x00603b9a, clamped below at word
// 0x00603b9c; the quotient (delta^2 >> 4) / (2 * ax) (16-bit idiv, guarded against overflow, else
// the cap) times word [0x00603854] >> 16, capped at word 0x00623e04. cx is negated for a negative
// delta, sign-extended into eax and passed through 0x00426100: eax = ecx = its result, edx and the
// flags are those of 0x00426100.
void __cdecl FUN_0042a533(gp4::Regs* r) {
    const Unknown_0042a533_Body* self = reinterpret_cast<const Unknown_0042a533_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042a533_Body* other = reinterpret_cast<const Unknown_0042a533_Body*>(static_cast<uintptr_t>(r->ebx));

    uint16_t ax;
    if ((self->Unknown9C & 2u) == 0u && (other->Unknown9C & 2u) == 0u) {
        ax = static_cast<uint16_t>(other->UnknownE2 - self->UnknownE2);
    } else {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0040dc8a, call);
        ax = static_cast<uint16_t>(call.eax);
    }

    if ((self->UnknownCC & 8u) != 0u) {
        ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b98));
    }
    ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b50));

    bool capped = false;
    int16_t factor = 0;
    if (static_cast<int16_t>(ax) <= 0) {
        const int64_t product = static_cast<int64_t>(static_cast<int16_t>(ax)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00623df6));
        const uint16_t offset = static_cast<uint16_t>(static_cast<uint64_t>(product) >> 10);
        const uint16_t delta = static_cast<uint16_t>(other->Unknown9A + offset - self->Unknown9A);
        GP4_GLOBAL(uint16_t, 0x00934dec) = delta;
        const int32_t square = static_cast<int32_t>(static_cast<int16_t>(delta)) * static_cast<int32_t>(static_cast<int16_t>(delta));
        const int32_t n = square >> 4;
        if (n < 0x1000) {
            factor = static_cast<int16_t>(n);
        } else {
            capped = true;
        }
    } else {
        ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b9a));
        const uint16_t floorValue = GP4_GLOBAL(uint16_t, 0x00603b9c);
        if (static_cast<int16_t>(ax) < static_cast<int16_t>(floorValue)) {
            ax = floorValue;
        }
        const uint16_t delta = static_cast<uint16_t>(other->Unknown9A - self->Unknown9A);
        GP4_GLOBAL(uint16_t, 0x00934dec) = delta;
        const int16_t divisor = static_cast<int16_t>(static_cast<uint16_t>(ax << 1));
        const int32_t square = static_cast<int32_t>(static_cast<int16_t>(delta)) * static_cast<int32_t>(static_cast<int16_t>(delta));
        const int32_t n = square >> 4;
        const int16_t high = static_cast<int16_t>(n >> 16);
        int16_t half = static_cast<int16_t>(divisor >> 1);
        if (half < 0) {
            half = static_cast<int16_t>(half + 1);
        }
        bool fits;
        if (half < 0) {
            fits = high < static_cast<int16_t>(-half);
        } else {
            fits = high < half;
        }
        if (fits) {
            factor = static_cast<int16_t>(n / static_cast<int32_t>(divisor));
        } else {
            capped = true;
        }
    }

    uint16_t cx;
    const uint16_t cap = GP4_GLOBAL(uint16_t, 0x00623e04);
    if (capped) {
        cx = cap;
    } else {
        const int32_t scaled = static_cast<int32_t>(factor) * static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00603854));
        cx = static_cast<uint16_t>(static_cast<uint32_t>(scaled) >> 16);
        if (static_cast<int16_t>(cx) > static_cast<int16_t>(cap)) {
            cx = cap;
        }
    }
    if (static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00934dec)) < 0) {
        cx = static_cast<uint16_t>(0u - cx);
    }

    const uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(cx)));
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = value;
    call.ecx = (r->ecx & 0xffff0000u) | cx;
    gp4::call_regs(0x00426100, call);

    r->eax = call.eax;
    r->ecx = call.eax;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0042a533, FUN_0042a533, "regs(eax:u32, ecx:u32, esi:ptr[0x225]:layout=car_0042a533, ebx:ptr[0x225]:layout=car_0042a533) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00426091+gap_0042a533")

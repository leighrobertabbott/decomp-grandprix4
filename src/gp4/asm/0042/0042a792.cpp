#include <gp4/gp4.h>

struct Unknown_0042a792_Body {
    uint8_t Unknown00[0x9a];
    int16_t Unknown9A;
    uint8_t Unknown9C;
    uint8_t Unknown9D[0xcc - 0x9d];
    uint8_t UnknownCC;
    uint8_t UnknownCD[0xd0 - 0xcd];
    uint32_t UnknownD0;
    uint8_t UnknownD4[0xe2 - 0xd4];
    uint16_t UnknownE2;
};

// FUNCTION: GP4 0x0042a792
// esi / ebx = the two bodies (sibling of 0x0042a6dc). ax = difference of their words +0xe2 (or the
// 0x0040dc8a value when either has bit 1 of +0x9c set), minus word 0x00603b98 when bit 3 of esi's
// +0xcc is set, minus word 0x00603b50. Not positive: cx = other.+0x9a + (ax * [0x00623df6] >> 10),
// and |cx*cx - self.+0xd0| >> 4 below 0x1000 is scaled by word 0x00603854 (high word, capped at
// word 0x00623e08), else the cap. Positive: the 16-bit ratio ((other.+0xd0 - self.+0xd0) >> 4) /
// (2 * max(ax - [0x00603b9a], [0x00603b9c])) when it fits (else the cap) is scaled the same way.
// Word 0x00934dec = cx - self.+0x9a gives the sign; the result goes through 0x00426100 and
// eax = ecx = its result (edx and the flags are its too).
void __cdecl FUN_0042a792(gp4::Regs* r) {
    const Unknown_0042a792_Body* self = reinterpret_cast<const Unknown_0042a792_Body*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042a792_Body* other = reinterpret_cast<const Unknown_0042a792_Body*>(static_cast<uintptr_t>(r->ebx));

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

    uint16_t cx = static_cast<uint16_t>(other->Unknown9A);
    if ((self->UnknownCC & 8u) != 0u) {
        ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b98));
    }
    ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b50));

    const int16_t cap = GP4_GLOBAL(int16_t, 0x00623e08);
    uint32_t edxHigh = r->edx & 0xffff0000u;
    uint16_t dx;
    bool scale;

    if (static_cast<int16_t>(ax) <= 0) {
        const int64_t product = static_cast<int64_t>(static_cast<int16_t>(ax)) * GP4_GLOBAL(int32_t, 0x00623df6);
        edxHigh = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32) & 0xffff0000u;
        const uint16_t offset = static_cast<uint16_t>(static_cast<uint64_t>(product) >> 10);
        cx = static_cast<uint16_t>(cx + offset);
        GP4_GLOBAL(uint16_t, 0x00934dec) = static_cast<uint16_t>(cx - static_cast<uint16_t>(self->Unknown9A));

        const int32_t square = static_cast<int32_t>(static_cast<int16_t>(cx)) * static_cast<int32_t>(static_cast<int16_t>(cx));
        uint32_t value = static_cast<uint32_t>(square) - self->UnknownD0;
        if (static_cast<int32_t>(value) < 0) {
            value = 0u - value;
        }
        value = static_cast<uint32_t>(static_cast<int32_t>(value) >> 4);
        ax = static_cast<uint16_t>(value);
        dx = static_cast<uint16_t>(value >> 16);
        scale = static_cast<int32_t>(value) < 0x1000;
    } else {
        ax = static_cast<uint16_t>(ax - GP4_GLOBAL(uint16_t, 0x00603b9a));
        if (static_cast<int16_t>(ax) < GP4_GLOBAL(int16_t, 0x00603b9c)) {
            ax = GP4_GLOBAL(uint16_t, 0x00603b9c);
        }
        GP4_GLOBAL(uint16_t, 0x00934dec) = static_cast<uint16_t>(cx - static_cast<uint16_t>(self->Unknown9A));
        cx = static_cast<uint16_t>(ax << 1);

        uint32_t distance = static_cast<uint32_t>(static_cast<int32_t>(other->UnknownD0 - self->UnknownD0) >> 4);
        if (static_cast<int32_t>(distance) < 0) {
            distance = 0u - distance;
        }
        ax = static_cast<uint16_t>(distance);
        dx = static_cast<uint16_t>(distance >> 16);

        int16_t half = static_cast<int16_t>(static_cast<int16_t>(cx) >> 1);
        if (half < 0) {
            half = static_cast<int16_t>(half + 1);
            half = static_cast<int16_t>(-half);
        }
        scale = static_cast<int16_t>(dx) < half;
        if (scale) {
            const int32_t dividend = static_cast<int32_t>(distance);
            const int32_t divisor = static_cast<int16_t>(cx);
            ax = static_cast<uint16_t>(dividend / divisor);
            dx = static_cast<uint16_t>(dividend % divisor);
        }
    }

    if (scale) {
        const int32_t product = static_cast<int32_t>(static_cast<int16_t>(ax)) * static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00603854));
        dx = static_cast<uint16_t>(static_cast<uint32_t>(product) >> 16);
        cx = dx;
        if (static_cast<int16_t>(cx) > cap) {
            cx = static_cast<uint16_t>(cap);
        }
    } else {
        cx = static_cast<uint16_t>(cap);
    }

    if ((GP4_GLOBAL(uint16_t, 0x00934dec) & 0x8000u) != 0u) {
        cx = static_cast<uint16_t>(0u - cx);
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(cx)));
    call.ecx = (r->ecx & 0xffff0000u) | cx;
    call.edx = edxHigh | dx;
    gp4::call_regs(0x00426100, call);

    r->eax = call.eax;
    r->ecx = call.eax;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0042a792, FUN_0042a792, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x225]:layout=car_0042a533+car_0042a792, ebx:ptr[0x225]:layout=car_0042a533+car_0042a792) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); cases=500; globals=fuzz:i32+layout=state_00426091+gap_0042a533")

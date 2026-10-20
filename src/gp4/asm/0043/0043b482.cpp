#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043b482
// Car lateral offset in 32 bits (dx:ax). Car +0x70 (minus one, or two when car byte +0x71 bit 1 is
// set and the segment angle is >= word 0x86856c) is multiplied by the wrap length word 0x868558;
// the 16-bit segment angle (segment word +0x1a & 0x7ff, or the 0x405985 split of it when car byte
// +0x8d bit 1 is set) is added. 0x934df0 is saved and restored around the use as scratch.
void __cdecl FUN_0043b482(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t saved = GP4_GLOBAL(uint32_t, 0x00934df0);
    uint8_t* segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, car, 0x1c)));
    const uint16_t angle = static_cast<uint16_t>(GP4_FIELD(uint16_t, segment, 0x1a) & 0x7ffu);
    const uint8_t carFlags71 = GP4_FIELD(uint8_t, car, 0x71);

    uint16_t ax = static_cast<uint16_t>(GP4_FIELD(uint8_t, car, 0x70) - 1);
    uint32_t eaxHigh = r->eax & 0xffff0000u;
    uint32_t edxHigh = r->edx & 0xffff0000u;

    if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) == 0u) {
        if ((carFlags71 & 2u) != 0u && static_cast<int16_t>(angle) >= GP4_GLOBAL(int16_t, 0x0086856c)) {
            ax = static_cast<uint16_t>(ax - 1u);
        }
        const int32_t product = static_cast<int32_t>(static_cast<int16_t>(ax)) * GP4_GLOBAL(int16_t, 0x00868558);
        GP4_GLOBAL(uint16_t, 0x00934df0) = static_cast<uint16_t>(product);
        GP4_GLOBAL(uint16_t, 0x00934df2) = static_cast<uint16_t>(static_cast<uint32_t>(product) >> 16);
        ax = angle;
    } else {
        const int32_t product = static_cast<int32_t>(static_cast<int16_t>(ax)) * GP4_GLOBAL(int16_t, 0x00868558);
        GP4_GLOBAL(uint16_t, 0x00934df0) = static_cast<uint16_t>(product);
        GP4_GLOBAL(uint16_t, 0x00934df2) = static_cast<uint16_t>(static_cast<uint32_t>(product) >> 16);

        // mov al,[esi+0x67]; cbw; xchg cx,ax -> cx = sign-extended byte, ax = angle
        gp4::Regs callee = *r;
        callee.st_in = 0;
        callee.st_out = 0;
        callee.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment));
        callee.eax = eaxHigh | angle;
        callee.ecx = (r->ecx & 0xffff0000u)
            | static_cast<uint16_t>(static_cast<int16_t>(GP4_FIELD(int8_t, car, 0x67)));
        callee.edx = edxHigh | static_cast<uint16_t>(static_cast<uint32_t>(product) >> 16);
        gp4::call_regs(0x00405985, callee);
        eaxHigh = callee.eax & 0xffff0000u;
        edxHigh = callee.edx & 0xffff0000u;
        ax = static_cast<uint16_t>(callee.eax);
        if ((carFlags71 & 2u) != 0u) {
            ax = static_cast<uint16_t>(ax - static_cast<uint16_t>(GP4_GLOBAL(int16_t, 0x00868558)));
        }
    }

    // cwd; add ax,[0x934df0]; adc dx,[0x934df2]
    const uint32_t dxIn = (ax & 0x8000u) != 0u ? 0xffffu : 0u;
    const uint32_t lowSum = static_cast<uint32_t>(ax) + GP4_GLOBAL(uint16_t, 0x00934df0);
    const uint32_t carryIn = lowSum >> 16;
    const uint32_t addend = GP4_GLOBAL(uint16_t, 0x00934df2);
    const uint32_t highSum = dxIn + addend + carryIn;
    const uint32_t highRes = highSum & 0xffffu;
    const uint32_t flags = parityFlag(highRes)
        | (highSum > 0xffffu ? 1u : 0u)
        | (((dxIn ^ addend ^ highRes) & 0x10u) != 0u ? 0x10u : 0u)
        | (highRes == 0u ? 0x40u : 0u)
        | ((highRes & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(dxIn ^ addend) & (dxIn ^ highRes) & 0x8000u) != 0u ? 0x800u : 0u);

    GP4_GLOBAL(uint32_t, 0x00934df0) = saved;

    r->eax = eaxHigh | (lowSum & 0xffffu);
    r->edx = edxHigh | highRes;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043b482, FUN_0043b482, "regs(eax:u32, ecx:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+lap_0043b482")

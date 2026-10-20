#include <gp4/gp4.h>

// Rotates the 2D vector held in the globals (0x934df0, 0x934df4) by the angle in
// the first word of the record at edi (Q14 cosine/sine table lookup), then adds
// the record's words at +4 and +8 scaled by 0x800. The cosine and the (negated)
// sine used are left in 0x934de8 / 0x934dec.

static inline uint32_t TableWord(uint32_t byteOffset) {
    return (uint32_t)(int32_t) * reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + byteOffset);
}

static inline uint32_t ParityFlag(uint32_t v) {
    uint32_t b = v & 0xffu;
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return (b & 1u) ? 0u : 4u;  // PF set when the low byte has an even number of 1 bits
}

// FUNCTION: GP4 0x004046d1
void __cdecl FUN_004046d1(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint16_t angle = *reinterpret_cast<const uint16_t*>(rec);

    uint16_t a = angle;
    if ((int16_t)a < 0) a = (uint16_t)(0u - a);
    a = (uint16_t)(a >> 2);
    const uint32_t cosv = TableWord((uint32_t)a & 0xfffeu);
    GP4_GLOBAL(uint32_t, 0x00934de8) = cosv;

    uint16_t b = (uint16_t)(0x4000u - angle);
    if ((int16_t)b < 0) b = (uint16_t)(0u - b);
    b = (uint16_t)(b >> 2);
    const uint32_t sinv = 0u - TableWord((uint32_t)b & 0xfffeu);
    GP4_GLOBAL(uint32_t, 0x00934dec) = sinv;

    const int32_t x = (int32_t)GP4_GLOBAL(uint32_t, 0x00934df0);
    const int32_t y = (int32_t)GP4_GLOBAL(uint32_t, 0x00934df4);

    const int64_t xs = (int64_t)x * (int64_t)(int32_t)sinv;
    const uint32_t newX = (uint32_t)(((int64_t)x * (int64_t)(int32_t)cosv) >> 14) -
                          (uint32_t)(((int64_t)y * (int64_t)(int32_t)sinv) >> 14);
    const uint32_t newY = (uint32_t)(((int64_t)y * (int64_t)(int32_t)cosv) >> 14) +
                          (uint32_t)(xs >> 14);

    GP4_GLOBAL(uint32_t, 0x00934df0) = newX + ((uint32_t)(int32_t)*reinterpret_cast<const int16_t*>(rec + 4) << 11);

    const uint32_t addend = (uint32_t)(int32_t)*reinterpret_cast<const int16_t*>(rec + 8) << 11;
    const uint32_t res = addend + newY;
    GP4_GLOBAL(uint32_t, 0x00934df4) = res;

    // flags of the final ADD (eax = addend, [mem] = newY)
    const uint32_t cf = res < addend ? 1u : 0u;
    const uint32_t af = ((addend ^ newY ^ res) >> 4) & 1u;
    const uint32_t zf = res == 0 ? 1u : 0u;
    const uint32_t sf = res >> 31;
    const uint32_t of = ((addend ^ res) & (newY ^ res)) >> 31;
    const uint32_t mask = 0x1u | 0x4u | 0x10u | 0x40u | 0x80u | 0x800u;
    r->eflags = (r->eflags & ~mask) | cf | ParityFlag(res) | (af << 4) | (zf << 6) | (sf << 7) | (of << 11);

    r->eax = res;
    r->edx = (uint32_t)((uint64_t)xs >> 32);
}
GP4_IMPL(0x004046d1, FUN_004046d1, "regs(edi:ptr[0x40]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

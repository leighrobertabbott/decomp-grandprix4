#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `add dest, src`.
static inline uint32_t addFlags32(uint32_t dest, uint32_t src) {
    const uint32_t sum = dest + src;
    return (sum < dest ? 1u : 0u)
        | parityFlag(sum)
        | ((dest ^ src ^ sum) & 0x10u)
        | (sum == 0u ? 0x40u : 0u)
        | ((sum >> 24) & 0x80u)
        | (((~(dest ^ src) & (dest ^ sum)) >> 20) & 0x800u);
}

static inline uint32_t withLow16(uint32_t entry, uint32_t low) {
    return (entry & 0xffff0000u) | (low & 0xffffu);
}

static inline gp4::Regs scratchRegs(const gp4::Regs& state) {
    gp4::Regs t = state;
    t.st_in = 0;
    t.st_out = 0;
    t.st_xin_mask = 0;
    t.st_xout_mask = 0;
    return t;
}

// Q14 table interpolation of a 16-bit angle by 0x0040119c (ax in, ax out).
static inline uint16_t tableLookup(const gp4::Regs& state, uint16_t angle) {
    gp4::Regs t = scratchRegs(state);
    t.eax = withLow16(state.eax, angle);
    gp4::call_regs(0x0040119c, t);
    return static_cast<uint16_t>(t.eax);
}

// (product << 2) >> 16 of a 16x16 imul, then cwd: the sign-extended Q14 word of the product.
static inline uint32_t q14Word(uint32_t product) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(static_cast<uint16_t>(product >> 14))));
}

static inline uint32_t imul16(uint16_t a, uint16_t b) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(a))
        * static_cast<int32_t>(static_cast<int16_t>(b)));
}

// 32x32 signed product through 0x00401000, kept as bits 30..61 (two shl/rcl steps over the
// upper 48 bits). The product's words go to [scratchHigh] (bits 32..63) and [low] (bits 0..31)
// first, then [scratchHigh] receives the scaled dword.
static inline uint32_t scaledProduct(const gp4::Regs& state, uint32_t aAddr, uint32_t bAddr) {
    gp4::Regs m = scratchRegs(state);
    m.eax = withLow16(state.eax, GP4_GLOBAL(uint16_t, aAddr));
    m.edx = withLow16(state.edx, GP4_GLOBAL(uint16_t, aAddr + 2u));
    m.ebx = withLow16(state.ebx, GP4_GLOBAL(uint16_t, bAddr));
    m.ecx = withLow16(state.ecx, GP4_GLOBAL(uint16_t, bAddr + 2u));
    gp4::call_regs(0x00401000, m);
    GP4_GLOBAL(uint16_t, 0x00934de0) = static_cast<uint16_t>(m.ebx);
    GP4_GLOBAL(uint16_t, 0x00934de2) = static_cast<uint16_t>(m.ecx);
    GP4_GLOBAL(uint16_t, bAddr) = static_cast<uint16_t>(m.eax);
    GP4_GLOBAL(uint16_t, bAddr + 2u) = static_cast<uint16_t>(m.edx);

    const uint64_t product = (static_cast<uint64_t>(GP4_GLOBAL(uint16_t, 0x00934de2)) << 48)
        | (static_cast<uint64_t>(GP4_GLOBAL(uint16_t, 0x00934de0)) << 32)
        | (static_cast<uint64_t>(GP4_GLOBAL(uint16_t, bAddr + 2u)) << 16);
    const uint32_t scaled = static_cast<uint32_t>(product >> 30);
    GP4_GLOBAL(uint32_t, 0x00934de0) = scaled;
    GP4_GLOBAL(uint32_t, bAddr) = scaled;
    return scaled;
}

// FUNCTION: GP4 0x0040a217
// esi = car record, [esi+0x1c] = its track segment record (left in edi). Word +0x92 of the car
// becomes the Q14 product of word +0x90 and the segment's word +0x14 (halved). The pair
// (+0x92, +0x90) is rotated by the segment heading (cos/sin of word +0 from the 0x0040119c table),
// offset by the segment origin words +4/+8 (<< 3, low bits from byte +0x21) and by the scale
// word [0x62ac58] along the car heading +0x18; the results go to [0x62ac6c] / [0x62ac70] (and
// 0x934df4 / 0x934df0). A nonzero dword [0x62ac64] then adds its magnitude times the sine/cosine
// pair (0x0040a006) of heading +-0x4000 (bits 30..61 of the 0x00401000 products).
void __cdecl FUN_0040a217(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t segAddr = GP4_FIELD(uint32_t, car, 0x1c);
    const uint8_t* seg = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(segAddr));
    const uint32_t edxHigh = r->edx & 0xffff0000u;

    const uint32_t p0 = imul16(GP4_FIELD(uint16_t, car, 0x90), *reinterpret_cast<const uint16_t*>(seg + 0x14));
    const uint16_t lateral = static_cast<uint16_t>(static_cast<int16_t>(static_cast<uint16_t>(p0 >> 14)) >> 1);
    GP4_FIELD(uint16_t, car, 0x92) = lateral;
    GP4_GLOBAL(uint16_t, 0x00934df0) = GP4_FIELD(uint16_t, car, 0x92);
    GP4_GLOBAL(uint16_t, 0x00934df4) = GP4_FIELD(uint16_t, car, 0x90);

    const uint16_t segAngle = *reinterpret_cast<const uint16_t*>(seg);
    const uint16_t segCos = tableLookup(*r, segAngle);
    const uint16_t segSin = tableLookup(*r, static_cast<uint16_t>(0x4000u - segAngle));
    GP4_GLOBAL(uint16_t, 0x00934de8) = segCos;
    GP4_GLOBAL(uint16_t, 0x00934dec) = segSin;

    const uint16_t u = GP4_GLOBAL(uint16_t, 0x00934df0);
    const uint16_t v = GP4_GLOBAL(uint16_t, 0x00934df4);
    const uint32_t rotX = imul16(u, segCos) - imul16(v, segSin);
    const uint32_t rotY = imul16(v, segCos) + imul16(u, segSin);

    const uint8_t lowBits = seg[0x21];
    uint32_t y = q14Word(rotY)
        + ((static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(seg + 4))) << 3)
            | (lowBits & 7u));
    uint32_t x = q14Word(rotX)
        + ((static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(seg + 8))) << 3)
            | (lowBits >> 4));

    const uint16_t heading = GP4_FIELD(uint16_t, car, 0x18);
    const uint16_t headSin = tableLookup(*r, static_cast<uint16_t>(0x4000u - heading));
    const uint16_t headCos = tableLookup(*r, heading);
    const uint16_t scale = GP4_GLOBAL(uint16_t, 0x0062ac58);

    y += q14Word(imul16(headSin, scale));
    GP4_GLOBAL(uint32_t, 0x00934df4) = y;
    GP4_GLOBAL(uint32_t, 0x0062ac6c) = y;
    x += q14Word(imul16(headCos, scale));
    GP4_GLOBAL(uint32_t, 0x00934df0) = x;
    GP4_GLOBAL(uint32_t, 0x0062ac70) = x;

    const uint32_t ecxOut = withLow16(r->ecx, scale);
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x0062ac64);
    GP4_GLOBAL(uint32_t, 0x00934de4) = offset;
    r->ecx = ecxOut;
    r->edi = segAddr;
    if (offset == 0u) {
        // mov ax,[0x62ac64] / mov dx,[0x62ac66] / or ax,dx: both words zero.
        r->eax = edxHigh;
        r->edx = edxHigh;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    const bool negative = (offset & 0x80000000u) != 0u;
    if (negative) {
        GP4_GLOBAL(uint32_t, 0x00934de4) = 0u - offset;
    }
    GP4_GLOBAL(uint32_t, 0x00934df0) = GP4_GLOBAL(uint32_t, 0x00934de4);
    const uint16_t sideAngle = negative
        ? static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x18) - 0x4000u)
        : static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x18) + 0x4000u);
    GP4_GLOBAL(uint16_t, 0x00934de8) = sideAngle;

    gp4::Regs pair = scratchRegs(*r);
    pair.eax = edxHigh | sideAngle;
    pair.edx = edxHigh | (offset >> 16);
    gp4::call_regs(0x0040a006, pair);

    const uint32_t addY = scaledProduct(pair, 0x00934dec, 0x00934de4);
    const uint32_t oldY = GP4_GLOBAL(uint32_t, 0x0062ac6c);
    GP4_GLOBAL(uint32_t, 0x0062ac6c) = oldY + addY;

    const uint32_t addX = scaledProduct(pair, 0x00934de8, 0x00934df0);
    const uint32_t oldX = GP4_GLOBAL(uint32_t, 0x0062ac70);
    GP4_GLOBAL(uint32_t, 0x0062ac70) = oldX + addX;

    r->eax = addX;
    r->ebx = pair.ebx;
    r->ecx = pair.ecx;
    r->edx = pair.edx;
    r->esi = pair.esi;
    r->edi = pair.edi;
    r->ebp = pair.ebp;
    r->eflags = (pair.eflags & ~0x8d5u) | addFlags32(oldX, addX);
}
GP4_IMPL(0x0040a217, FUN_0040a217, "regs(ecx:u32, edx:u32, esi:ptr[0x420]:layout=car_0042ba87) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

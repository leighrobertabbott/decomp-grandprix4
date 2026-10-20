#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the three-operand `imul r32, r32, imm` (oracle model): CF = OF = the product does not
// fit 32 bits, AF = 0, PF/ZF/SF from the low dword.
static inline uint32_t imulFlags32(int32_t value, int32_t factor) {
    const int64_t product = static_cast<int64_t>(value) * factor;
    const uint32_t low = static_cast<uint32_t>(product);
    const bool overflow = product != static_cast<int64_t>(static_cast<int32_t>(low));
    return (overflow ? 0x801u : 0u)
        | parityFlag(low)
        | (low == 0u ? 0x40u : 0u)
        | ((low >> 24) & 0x80u);
}

static inline uint16_t word(const uint8_t* p, uint32_t off) {
    return *reinterpret_cast<const uint16_t*>(p + off);
}

static inline uint32_t dword(const uint8_t* p, uint32_t off) {
    return *reinterpret_cast<const uint32_t*>(p + off);
}

// (signed word - 5000) * 8 / 100, truncated toward zero; the low byte is stored.
static inline uint8_t centredPercent(const uint8_t* p) {
    const int32_t value = static_cast<int32_t>(*reinterpret_cast<const int16_t*>(p)) - 0x1388;
    return static_cast<uint8_t>((value * 8) / 100);
}

// FUNCTION: GP4 0x0040c6d8
// Unpacks a 0xdc-byte data block at esi into the globals: four runs of 8 bytes (each minus 0x97) into
// the template rows at 0x7ad9f4 (+0/+0x70, +0x38/+0xa8) and 0x62d1b8/0x62d1c8, two centred words
// ((w - 5000) * 8 / 100) into 0x62d1c1/0x62d1d1, two bytes minus 0x34, a word compared with 50 (flag
// byte 0x62d27c and the template bytes +8/+0x32), 36 words, dwords and words into 0x62d1f2..0x62d2b8
// and six dwords into 0x62d25c. Then derived values: [0x602a4c], [0x602a9c], [0x62d28c], the rescaled
// word [0x62d1fa], 0xf0000 / the per-circuit byte at 0x62eade + [0x7ad894] into [0x62d258], and four
// dwords scaled by 100 / 20. esi is left past the block.
void __cdecl FUN_0040c6d8(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const rows = GP4_ARRAY(uint8_t, 0x007ad9f4);

    for (uint32_t i = 0; i < 8u; ++i) {
        const uint8_t value = static_cast<uint8_t>(src[i] - 0x97u);
        rows[i] = value;
        rows[0x70u + i] = value;
    }
    for (uint32_t i = 0; i < 8u; ++i) {
        const uint8_t value = static_cast<uint8_t>(src[8u + i] - 0x97u);
        rows[0x38u + i] = value;
        rows[0xa8u + i] = value;
    }
    for (uint32_t i = 0; i < 8u; ++i) {
        GP4_ARRAY(uint8_t, 0x0062d1b8)[i] = static_cast<uint8_t>(src[0x10u + i] - 0x97u);
    }
    GP4_GLOBAL(uint8_t, 0x0062d1c1) = centredPercent(src + 0x18);
    for (uint32_t i = 0; i < 8u; ++i) {
        GP4_ARRAY(uint8_t, 0x0062d1c8)[i] = static_cast<uint8_t>(src[0x1cu + i] - 0x97u);
    }
    GP4_GLOBAL(uint8_t, 0x0062d1d1) = centredPercent(src + 0x24);
    GP4_GLOBAL(uint32_t, 0x0062d1d8) = static_cast<uint32_t>(src[0x28]) - 0x34u;
    GP4_GLOBAL(uint32_t, 0x0062d1dc) = static_cast<uint32_t>(src[0x29]) - 0x34u;
    GP4_GLOBAL(uint16_t, 0x0062d1f0) = word(src, 0x2a);

    const uint8_t shortFlag = GP4_GLOBAL(int16_t, 0x0062d1f0) < 0x32 ? 1u : 0u;
    GP4_GLOBAL(uint8_t, 0x0062d27c) = shortFlag;
    rows[8] = shortFlag;
    rows[0x32] = shortFlag;
    rows[0x70 + 8] = shortFlag;
    rows[0x70 + 0x32] = shortFlag;

    GP4_GLOBAL(uint32_t, 0x0062d1e0) = 4u;
    GP4_GLOBAL(uint32_t, 0x0062d1e4) = 5u;
    GP4_GLOBAL(uint32_t, 0x0062d1e8) = 6u;
    GP4_GLOBAL(uint32_t, 0x0062d1ec) = 7u;

    const uint8_t* b = src + 0x2c;
    GP4_GLOBAL(uint16_t, 0x0062d1f2) = word(b, 0x00);
    GP4_GLOBAL(uint16_t, 0x0062d1f4) = word(b, 0x02);
    GP4_GLOBAL(uint16_t, 0x0062d1f6) = word(b, 0x04);
    GP4_GLOBAL(uint16_t, 0x0062d1f8) = word(b, 0x06);
    GP4_GLOBAL(uint16_t, 0x0062d1fa) = word(b, 0x08);
    GP4_GLOBAL(uint16_t, 0x0062d1fc) = word(b, 0x0a);
    GP4_GLOBAL(uint16_t, 0x0062d1fe) = word(b, 0x0c);
    GP4_GLOBAL(uint16_t, 0x0062d200) = word(b, 0x0e);
    GP4_GLOBAL(uint16_t, 0x0062d202) = word(b, 0x10);
    GP4_GLOBAL(uint16_t, 0x0062d204) = word(b, 0x12);
    GP4_GLOBAL(uint16_t, 0x0062d206) = word(b, 0x14);
    GP4_GLOBAL(uint16_t, 0x0062d208) = word(b, 0x16);
    GP4_GLOBAL(uint16_t, 0x0062d20a) = word(b, 0x18);
    GP4_GLOBAL(uint16_t, 0x0062d20c) = word(b, 0x1a);
    GP4_GLOBAL(uint16_t, 0x0062d20e) = word(b, 0x1c);
    GP4_GLOBAL(uint16_t, 0x0062d218) = word(b, 0x1e);
    GP4_GLOBAL(uint16_t, 0x0062d222) = word(b, 0x20);
    GP4_GLOBAL(uint16_t, 0x0062d216) = word(b, 0x22);
    GP4_GLOBAL(uint16_t, 0x0062d220) = word(b, 0x24);
    GP4_GLOBAL(uint16_t, 0x0062d214) = word(b, 0x26);
    GP4_GLOBAL(uint16_t, 0x0062d21e) = word(b, 0x28);
    GP4_GLOBAL(uint16_t, 0x0062d212) = word(b, 0x2a);
    GP4_GLOBAL(uint16_t, 0x0062d21c) = word(b, 0x2c);
    GP4_GLOBAL(uint16_t, 0x0062d210) = word(b, 0x2e);
    GP4_GLOBAL(uint16_t, 0x0062d21a) = word(b, 0x30);
    GP4_GLOBAL(uint16_t, 0x0062d224) = word(b, 0x32);
    GP4_GLOBAL(uint16_t, 0x0062d226) = word(b, 0x34);
    GP4_GLOBAL(uint16_t, 0x0062d228) = word(b, 0x36);
    GP4_GLOBAL(uint16_t, 0x0062d22a) = word(b, 0x38);
    GP4_GLOBAL(uint16_t, 0x0062d22e) = word(b, 0x3a);
    GP4_GLOBAL(uint16_t, 0x0062d230) = word(b, 0x3c);
    GP4_GLOBAL(uint16_t, 0x0062d22c) = word(b, 0x3e);
    GP4_GLOBAL(uint16_t, 0x0062d232) = word(b, 0x40);
    GP4_GLOBAL(uint16_t, 0x0062d234) = word(b, 0x42);
    GP4_GLOBAL(uint16_t, 0x0062d236) = word(b, 0x44);
    GP4_GLOBAL(uint16_t, 0x0062d254) = word(b, 0x46);
    GP4_GLOBAL(uint32_t, 0x0062d238) = dword(b, 0x48);
    GP4_GLOBAL(uint32_t, 0x0062d23c) = dword(b, 0x4c);
    GP4_GLOBAL(uint32_t, 0x0062d240) = dword(b, 0x50);
    GP4_GLOBAL(uint32_t, 0x0062d244) = dword(b, 0x54);
    GP4_GLOBAL(uint16_t, 0x0062d248) = word(b, 0x58);
    GP4_GLOBAL(uint16_t, 0x0062d24a) = word(b, 0x5a);
    GP4_GLOBAL(uint16_t, 0x0062d24c) = word(b, 0x5c);
    GP4_GLOBAL(uint16_t, 0x0062d24e) = word(b, 0x5e);
    GP4_GLOBAL(uint32_t, 0x0062d250) = dword(b, 0x60);
    GP4_GLOBAL(uint16_t, 0x0062d256) = word(b, 0x64);
    GP4_GLOBAL(uint32_t, 0x0062d274) = dword(b, 0x68);
    GP4_GLOBAL(uint32_t, 0x0062d278) = dword(b, 0x6c);
    GP4_GLOBAL(uint32_t, 0x0062d294) = dword(b, 0x70);
    GP4_GLOBAL(uint32_t, 0x0062d29c) = dword(b, 0x74);
    GP4_GLOBAL(uint32_t, 0x0062d2a0) = dword(b, 0x78);
    GP4_GLOBAL(uint32_t, 0x0062d298) = dword(b, 0x7c);
    GP4_GLOBAL(uint32_t, 0x0062d2a4) = dword(b, 0x80);
    GP4_GLOBAL(uint32_t, 0x0062d2a8) = dword(b, 0x84);
    GP4_GLOBAL(uint32_t, 0x0062d2ac) = dword(b, 0x88);
    GP4_GLOBAL(uint32_t, 0x0062d2b0) = dword(b, 0x8c);
    GP4_GLOBAL(uint32_t, 0x0062d2b4) = dword(b, 0x90);
    GP4_GLOBAL(uint32_t, 0x0062d2b8) = dword(b, 0x94);
    for (uint32_t i = 0; i < 6u; ++i) {
        GP4_ARRAY(uint32_t, 0x0062d25c)[i] = dword(b, 0x98u + 4u * i);
    }

    GP4_GLOBAL(uint32_t, 0x00602a4c) = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0062d206)) << 8;

    const int64_t gripProduct = static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x0062d20a)))
        * GP4_GLOBAL(int32_t, 0x00602a98);
    GP4_GLOBAL(uint32_t, 0x00602a9c) = static_cast<uint32_t>(static_cast<uint64_t>(gripProduct) >> 14);

    // mov eax,0x12a; imul edx; shld edx,eax,14; shl eax,14; idiv ((w + 0x111) * 0x3de)
    const int32_t numerator = 0x12a * static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x0062d202));
    const int32_t divisor = (static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x0062d200)) + 0x111) * 0x3de;
    int32_t remainder;
    GP4_GLOBAL(int32_t, 0x0062d28c) = gp4::idiv64(static_cast<int64_t>(numerator) << 14, divisor, &remainder);

    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x0062d1fa)))
        * GP4_GLOBAL(int32_t, 0x00625a6c);
    GP4_GLOBAL(uint16_t, 0x0062d1fa) = static_cast<uint16_t>(static_cast<uint64_t>(scaled) >> 14);

    const uint32_t circuit = GP4_GLOBAL(uint32_t, 0x007ad894);
    const uint32_t circuitByte = GP4_ARRAY(const uint8_t, 0x0062eade)[circuit];
    uint32_t rate;
    GP4_GLOBAL(uint32_t, 0x0062d258) = gp4::div64(0xf0000u, circuitByte, &rate);

    GP4_GLOBAL(uint32_t, 0x0062d294) = GP4_GLOBAL(uint32_t, 0x0062d294) * 100u;
    GP4_GLOBAL(uint32_t, 0x0062d29c) = GP4_GLOBAL(uint32_t, 0x0062d29c) * 20u;
    GP4_GLOBAL(uint32_t, 0x0062d2a0) = GP4_GLOBAL(uint32_t, 0x0062d2a0) * 100u;
    const int32_t last = GP4_GLOBAL(int32_t, 0x0062d298);
    const uint32_t lastScaled = static_cast<uint32_t>(last) * 20u;
    GP4_GLOBAL(uint32_t, 0x0062d298) = lastScaled;

    r->eax = lastScaled;
    r->ebx = 0x007ad9f4u + 0x70u;
    r->ecx = 6u;
    r->edx = rate;
    r->esi = r->esi + 0xdcu;
    r->ebp = circuitByte;
    r->eflags = (r->eflags & ~0x8d5u) | imulFlags32(last, 20);
}
GP4_IMPL(0x0040c6d8, FUN_0040c6d8, "regs(esi:ptr[0xe0]:bytes) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=layout=trk_007ad894")

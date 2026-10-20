#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Signed 32x32 -> 64 multiplication (one-operand IMUL: edx:eax).
static inline uint64_t imul64(uint32_t left, uint32_t right) {
    return static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(left)) * static_cast<int64_t>(static_cast<int32_t>(right)));
}

// SHRD eax, edx, count: the low 32 bits of edx:eax >> count.
static inline uint32_t shiftRight(uint64_t product, unsigned count) {
    return static_cast<uint32_t>(product >> count);
}

// The two dword rows of constants at 0x00416660 (+0) and 0x00416668 (+8), picked by the byte offset 0 / 4.
static __declspec(noinline) int32_t constantNear(uint32_t offset) {
    return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x00416660u + offset));
}
static __declspec(noinline) int32_t constantFar(uint32_t offset) {
    return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x00416668u + offset));
}

// 0x00401315: eax = 16-bit angle (sign-extended) of the vector (edx, eax); everything else is preserved.
static inline uint32_t angleOf(const gp4::Regs* r, uint32_t first, uint32_t second) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = first;
    c.edx = second;
    gp4::call_regs(0x00401315, c);
    return c.eax;
}

// 0x0040119c: ax = table value of the 16-bit angle in ax; the upper half of eax is kept.
static inline uint32_t tableValue(const gp4::Regs* r, uint32_t eax) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    gp4::call_regs(0x0040119c, c);
    return c.eax;
}

// ebx = a 0x300-byte record (the first two of the four at 0x6380d8 use the first table row, the others the
// second one, selected by ebx >= 0x6386d8); eax = a Q14 factor. Builds two rotated components
//   ecx = ((((eax * [0x6034a8]) >> 14) << 14) + record.+0x1bc) * 0x168 >> 22) * record.+0x1b4 >> 14
//   ebp = table[row + 2] * record.+0x1b0 >> 28
// and from their angles (0x401315) a final angle whose magnitude (scaled by table[row]) sets bit 7 of the
// record byte +0x2e when it reaches 0x4000 (cleared before). eax returns
//   (((table(0x4000 - angle) * record.+0x1b0 >> 20) + (record.+0x1c0 >> 6)) * record.+0x1d8) >> 14
// with edx the high word of that last product and the flags those of its SHRD. DAT_0060344c keeps
// (eax * [0x6034a8]) >> 14. edi is saved and restored.
// FUNCTION: GP4 0x004167da
void __cdecl FUN_004167da(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    GP4_FIELD(uint8_t, record, 0x2e) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x2e) & 0x7fu);

    const uint32_t row = r->ebx < 0x006386d8u ? 0u : 4u;

    uint32_t value = shiftRight(imul64(r->eax, GP4_GLOBAL(uint32_t, 0x006034a8)), 14);
    GP4_GLOBAL(uint32_t, 0x0060344c) = value;
    value <<= 14;
    value += GP4_FIELD(uint32_t, record, 0x1bc);
    value = shiftRight(imul64(value, 0x168u), 22);
    const uint32_t ecx = shiftRight(imul64(value, GP4_FIELD(uint32_t, record, 0x1b4)), 14);
    const uint32_t ebp = shiftRight(imul64(static_cast<uint32_t>(constantFar(row)), GP4_FIELD(uint32_t, record, 0x1b0)), 28);

    value = angleOf(r, ecx, ebp);
    value = shiftRight(imul64(value, GP4_GLOBAL(uint32_t, 0x006f2bec)), 14);
    value = shiftRight(imul64(value, ebp), 14);
    value = 0u - value + ecx;
    value = shiftRight(imul64(value, GP4_FIELD(uint32_t, record, 0x1b8)), 14);
    value = 0u - value + ecx;
    value = angleOf(r, value, ebp);
    value = shiftRight(imul64(value, static_cast<uint32_t>(constantNear(row))), 14);

    uint32_t magnitude = value;
    if (static_cast<int32_t>(magnitude) < 0) {
        magnitude = 0u - magnitude;
    }
    if (static_cast<int32_t>(magnitude) >= 0x4000) {
        GP4_FIELD(uint8_t, record, 0x2e) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x2e) | 0x80u);
    }

    // neg ax; add ax, 0x4000
    value = (value & 0xffff0000u) | static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(value));
    value = tableValue(r, value);
    value = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(value)));
    value = shiftRight(imul64(value, GP4_FIELD(uint32_t, record, 0x1b0)), 20);
    value += static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(uint32_t, record, 0x1c0)) >> 6);

    const uint64_t product = imul64(value, GP4_FIELD(uint32_t, record, 0x1d8));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(product >> 32);
    const uint32_t result = shiftRight(product, 14);
    const uint32_t previous = shiftRight(product, 13);
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->ecx = ecx;
    r->edx = high;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004167da, FUN_004167da, "regs(eax:i32, ebx:rec[0x6380d8,0x300,0..3]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_004167da")

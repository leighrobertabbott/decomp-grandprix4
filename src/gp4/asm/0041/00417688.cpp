#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `a + b`.
static inline uint32_t addFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a + b;
    uint32_t f = parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & 0x80000000u) != 0u ? 0x80u : 0u);
    if (res < a) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if ((~(a ^ b) & (a ^ res) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

static inline int32_t wordShifted(const uint8_t* record, uint32_t offset) {
    return static_cast<int32_t>(*reinterpret_cast<const int16_t*>(record + offset)) * 1024;
}

// One axis of the corner table at ebx = 0x006380d8: the start s = word(start)*1024 goes to +0x2c4/+0x5c4 (+0x8c4/+0xbc4);
// the end e = word(end)*1024 is limited so that it is at most |limit|*1024 away from s, keeping the sign of e - s, and the
// clamped end s + (+-min(|e - s|, |limit*1024|)) goes to +0x2c8/+0x5c8 (+0x8c8/+0xbc8). Leaves eax = e - s,
// ebp = |e - s|, edx = the clamped end and the flags of the final add.
struct Axis {
    uint32_t difference;
    uint32_t magnitude;
    uint32_t end;
    uint32_t flags;
};
static inline Axis clampAxis(uint8_t* table, const uint8_t* record, uint32_t endWord, uint32_t startWord,
                             uint32_t limitWord, uint32_t startSlot, uint32_t endSlot) {
    Axis axis;
    const uint32_t end = static_cast<uint32_t>(wordShifted(record, endWord));
    const uint32_t start = static_cast<uint32_t>(wordShifted(record, startWord));
    *reinterpret_cast<uint32_t*>(table + startSlot) = start;
    *reinterpret_cast<uint32_t*>(table + startSlot + 0x300) = start;
    axis.difference = end - start;
    axis.magnitude = static_cast<int32_t>(axis.difference) < 0 ? 0u - axis.difference : axis.difference;
    uint32_t limit = static_cast<uint32_t>(wordShifted(record, limitWord));
    if (static_cast<int32_t>(limit) < 0) limit = 0u - limit;
    if (static_cast<int32_t>(limit) > static_cast<int32_t>(axis.magnitude)) limit = axis.magnitude;
    if (static_cast<int32_t>(axis.difference) < 0) limit = 0u - limit;
    const uint32_t base = *reinterpret_cast<const uint32_t*>(table + startSlot);
    axis.flags = addFlags32(limit, base);
    axis.end = limit + base;
    *reinterpret_cast<uint32_t*>(table + endSlot) = axis.end;
    *reinterpret_cast<uint32_t*>(table + endSlot + 0x300) = axis.end;
    return axis;
}

// Builds the two interpolation columns of the 0x006380d8 table from the six words at esi+0x278..0x282.
// FUNCTION: GP4 0x00417688
void __cdecl FUN_00417688(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->ebx = 0x006380d8u;
    uint8_t* table = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    clampAxis(table, record, 0x280, 0x27e, 0x282, 0x2c4, 0x2c8);
    const Axis second = clampAxis(table, record, 0x27a, 0x278, 0x27c, 0x8c4, 0x8c8);

    r->eax = second.difference;
    r->ebp = second.magnitude;
    r->edx = second.end;
    r->eflags = (r->eflags & ~0x8d5u) | second.flags;
}
GP4_IMPL(0x00417688, FUN_00417688, "regs(esi:ptr[0x28a]:bytes) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

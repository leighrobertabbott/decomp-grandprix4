#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `left - right`.
static inline uint32_t subtractionFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Byte offset into the Q14 word table at 0x007c5c5c for a signed 16-bit angle: |angle| >> 2, even.
static inline uint32_t tableOffset(uint16_t angle) {
    if (static_cast<int16_t>(angle) < 0) {
        angle = static_cast<uint16_t>(0u - angle);
    }
    angle = static_cast<uint16_t>(angle >> 2);
    return angle & 0xfffeu;
}

// imul ; shrd eax, edx, 14: low dword of (a * b) >> 14.
static inline uint32_t mulShr14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00427e7c
// edi = block (words at +4, +8, +0xa, +0x60), esi = vector (dwords at +0, +4, word at +0x18).
// Builds the offset vector [0x623e30], [0x623e38] from the table values at angle [edi+0xa] (ebp from
// 0x4000 - angle, ecx from angle) scaled by min(0x100 - word [edi+0x60], DAT_00624820), the words at
// +4 / +8 (times 8) and the dwords at [esi] / [esi+4] (>> 8), then returns in eax the angle of that
// vector (0x00401315) minus word [esi+0x18], sign-extended. The flags are those of the 16-bit sub;
// ecx, ebp and edx are left holding the table values and the vector's y component.
void __cdecl FUN_00427e7c(gp4::Regs* r) {
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* vector = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x007c5c5c);

    const uint16_t angle = GP4_FIELD(uint16_t, block, 0xa);
    const int32_t ebp = static_cast<int16_t>(*reinterpret_cast<const uint16_t*>(table + tableOffset(static_cast<uint16_t>(0x4000u - angle))));
    const int32_t ecx = static_cast<int16_t>(*reinterpret_cast<const uint16_t*>(table + tableOffset(angle)));

    int32_t scale = 0x100 - static_cast<int32_t>(GP4_FIELD(uint16_t, block, 0x60));
    const int32_t limit = GP4_GLOBAL(int32_t, 0x00624820);
    if (scale > limit) {
        scale = limit;
    }

    uint32_t x = mulShr14(scale, ecx) + (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, block, 4))) << 3);
    GP4_GLOBAL(uint32_t, 0x00623e30) = x;
    uint32_t y = (0u - mulShr14(scale, ebp)) + (static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, block, 8))) << 3);
    GP4_GLOBAL(uint32_t, 0x00623e38) = y;

    x = (0u - static_cast<uint32_t>(GP4_FIELD(int32_t, vector, 0) >> 8)) + GP4_GLOBAL(uint32_t, 0x00623e30);
    GP4_GLOBAL(uint32_t, 0x00623e30) = x;
    y = (0u - static_cast<uint32_t>(GP4_FIELD(int32_t, vector, 4) >> 8)) + GP4_GLOBAL(uint32_t, 0x00623e38);
    GP4_GLOBAL(uint32_t, 0x00623e38) = y;

    gp4::Regs direction = *r;
    direction.st_in = 0;
    direction.st_out = 0;
    direction.eax = x;
    direction.edx = y;
    gp4::call_regs(0x00401315, direction);

    const uint16_t heading = static_cast<uint16_t>(direction.eax);
    const uint16_t reference = GP4_FIELD(uint16_t, vector, 0x18);
    const uint16_t difference = static_cast<uint16_t>(heading - reference);

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(difference)));
    r->ecx = static_cast<uint32_t>(ecx);
    r->edx = y;
    r->ebp = static_cast<uint32_t>(ebp);
    r->eflags = (r->eflags & ~0x8d5u) | subtractionFlags16(heading, reference);
}
GP4_IMPL(0x00427e7c, FUN_00427e7c, "regs(edi:ptr[0x68]:bytes, esi:ptr[0x20]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

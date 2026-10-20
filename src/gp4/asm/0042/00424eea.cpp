#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of "neg reg" (a subtraction 0 - value)
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// 0x78-byte records at 0x0063bd7c (only the fields this routine writes).
struct Unknown_0063bd7c {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    int32_t Unknown0C;
    int32_t Unknown10;
    int32_t Unknown14;
    int32_t Unknown18;
    int32_t Unknown1C;
    int32_t Unknown20;
    int32_t Unknown24;
    int32_t Unknown28;
    int32_t Unknown2C;
    uint8_t Unknown30[0x34];
    int32_t Unknown64;
    uint8_t Unknown68[0x78 - 0x68];
};
static_assert(sizeof(Unknown_0063bd7c) == 0x78, "record stride");

// FUNCTION: GP4 0x00424eea
// Copies one car record (esi; byte +0x25e is its slot, 0xff = none) into slot `slot` of the
// 0x78-byte records at 0x0063bd7c after 0x00424fab has rotated its position into
// DAT_00623e30/34/38: position (+0 = x + DAT_00623e30, +4 = z + DAT_00623e34, +8 = y +
// DAT_00623e38), three direct dwords, three velocity dwords >> 8, the segment offset (+0x64 =
// [esi+0x1c] - 0x008943a0) and, when bit 0 of [esi+0xe5] is set, three more dwords (+0x24..+0x2c);
// otherwise +0x24 and +0x28 are cleared. Leaves ebx = the slot record, edx from the callee and eax / the
// flags of the last instruction of the taken path. An empty slot (0xff) leaves ebx = 0xff and
// the flags of "cmp bl, 0xff".
void __cdecl FUN_00424eea(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t slot = GP4_FIELD(uint8_t, car, 0x25e);

    if (slot == 0xffu) {
        r->ebx = slot;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    Unknown_0063bd7c* const out = GP4_ARRAY(Unknown_0063bd7c, 0x0063bd7c) + slot;

    gp4::Regs call = *r;
    call.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    gp4::call_regs(0x00424fab, call);

    out->Unknown00 = GP4_FIELD(int32_t, car, 0x00) + GP4_GLOBAL(int32_t, 0x00623e30);
    out->Unknown04 = GP4_FIELD(int32_t, car, 0x08) + GP4_GLOBAL(int32_t, 0x00623e34);
    out->Unknown08 = GP4_FIELD(int32_t, car, 0x04) + GP4_GLOBAL(int32_t, 0x00623e38);
    out->Unknown0C = GP4_FIELD(int32_t, car, 0x10);
    out->Unknown10 = GP4_FIELD(int32_t, car, 0x0c);
    out->Unknown14 = GP4_FIELD(int32_t, car, 0x14);
    out->Unknown18 = GP4_FIELD(int32_t, car, 0xa4) >> 8;
    out->Unknown1C = GP4_FIELD(int32_t, car, 0x150) >> 8;
    out->Unknown20 = GP4_FIELD(int32_t, car, 0xa8) >> 8;
    out->Unknown64 = static_cast<int32_t>(GP4_FIELD(uint32_t, car, 0x1c) - 0x008943a0u);

    uint32_t flags;
    if ((GP4_FIELD(uint8_t, car, 0xe5) & 1u) == 0u) {
        out->Unknown24 = 0;
        const uint32_t value = GP4_FIELD(uint32_t, car, 0xac);
        // the original stores -value to +0x28 and immediately overwrites it with 0
        out->Unknown28 = 0;
        call.eax = 0u - value;
        flags = negFlags(value);
    } else {
        out->Unknown24 = GP4_FIELD(int32_t, car, 0x154);
        out->Unknown28 = GP4_FIELD(int32_t, car, 0x158);
        const int32_t last = GP4_FIELD(int32_t, car, 0x15c);
        out->Unknown2C = last;
        call.eax = static_cast<uint32_t>(last);
        flags = 0u;   // test byte, 1 with a result of 1: no flag set
    }
    call.eflags = (r->eflags & ~0x8d5u) | flags;
    *r = call;
}
GP4_IMPL(0x00424eea, FUN_00424eea, "regs(esi:ptr[0x266]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

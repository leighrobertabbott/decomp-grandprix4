#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The 0xd0-byte orientation body at 0x00622cf8 that 0x00401f02 rotates: the vector in (+0xb8..+0xc8) goes
// through the matrix at +0x58 and comes out at +0xa0..+0xb0.
struct Unknown_00622cf8_Body {
    uint8_t Unknown00[0xa0];
    double Unknown0A0;
    double Unknown0A8;
    double Unknown0B0;
    double Unknown0B8;
    double Unknown0C0;
    double Unknown0C8;
};

// One 0x78-byte entry of the array at 0x0063bd7c.
struct Unknown_0063bd7c_Entry {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    int32_t Unknown0C;
    int32_t Unknown10;
    int32_t Unknown14;
    int32_t Unknown18;
    int32_t Unknown1C;
    int32_t Unknown20;
    uint8_t Unknown24[0x78 - 0x24];
};

struct Unknown_0041d795_Source {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    int32_t Unknown0C;
    int32_t Unknown10;
    int32_t Unknown14;
    uint8_t Unknown18[0x25e - 0x18];
    uint8_t Unknown25E;
};

static void rotate(const gp4::Regs* r, uint32_t body) {
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.edx = body;
    gp4::call_regs(0x00401f02, callee);
}

// FUNCTION: GP4 0x0041d795
// pushal/popal wrapper (every register is restored). esi = a record; its byte +0x25e selects the 0x78-byte
// entry of the array at 0x0063bd7c. The entry's +0xc/+0x10/+0x14 get the record's +0x10/+0xc/+0x14. The three
// ints 0x638040/44/48 are rotated by the matrix of the body 0x00622cf8 (0x401f02), rounded into
// 0x623e30/34/38 and added to the record's +0 / the int 0x637fe4 / the record's +4 to give the entry's
// +0/+4/+8; the entry's ints +0x18/+0x1c/+0x20 are rotated in place the same way. The flags are those of
// the last add (popal does not restore them).
void __cdecl FUN_0041d795(gp4::Regs* r) {
    const Unknown_0041d795_Source* source = reinterpret_cast<const Unknown_0041d795_Source*>(static_cast<uintptr_t>(r->esi));
    Unknown_0063bd7c_Entry* entry = reinterpret_cast<Unknown_0063bd7c_Entry*>(
        static_cast<uintptr_t>(0x0063bd7cu + static_cast<uint32_t>(source->Unknown25E) * 0x78u));
    Unknown_00622cf8_Body* body = reinterpret_cast<Unknown_00622cf8_Body*>(static_cast<uintptr_t>(0x00622cf8u));

    entry->Unknown0C = source->Unknown10;
    entry->Unknown10 = source->Unknown0C;
    entry->Unknown14 = source->Unknown14;

    body->Unknown0B8 = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638040));
    body->Unknown0C0 = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638044));
    body->Unknown0C8 = static_cast<double>(GP4_GLOBAL(int32_t, 0x00638048));
    rotate(r, 0x00622cf8u);

    GP4_GLOBAL(int32_t, 0x00623e30) = gp4::x87::fistp32(body->Unknown0A0);
    GP4_GLOBAL(int32_t, 0x00623e34) = gp4::x87::fistp32(body->Unknown0A8);
    GP4_GLOBAL(int32_t, 0x00623e38) = gp4::x87::fistp32(body->Unknown0B0);

    entry->Unknown00 = GP4_GLOBAL(int32_t, 0x00623e30) + source->Unknown00;
    entry->Unknown04 = GP4_GLOBAL(int32_t, 0x00623e34) + GP4_GLOBAL(int32_t, 0x00637fe4);
    const uint32_t left = GP4_GLOBAL(uint32_t, 0x00623e38);
    const uint32_t right = static_cast<uint32_t>(source->Unknown04);
    entry->Unknown08 = static_cast<int32_t>(left + right);
    const uint32_t flags = additionFlags32(left, right);

    body->Unknown0B8 = static_cast<double>(entry->Unknown18);
    body->Unknown0C0 = static_cast<double>(entry->Unknown1C);
    body->Unknown0C8 = static_cast<double>(entry->Unknown20);
    rotate(r, 0x00622cf8u);

    entry->Unknown18 = gp4::x87::fistp32(body->Unknown0A0);
    entry->Unknown1C = gp4::x87::fistp32(body->Unknown0A8);
    entry->Unknown20 = gp4::x87::fistp32(body->Unknown0B0);

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041d795, FUN_0041d795, "regs(esi:ptr[0x266]:layout=car_0041d795) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+objv_0063bd7c")

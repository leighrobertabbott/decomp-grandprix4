#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of shl value, 5 (pinned oracle model: AF=0, CF=bit 27, OF=bit31(prev ^ result))
static inline uint32_t shl5Flags(uint32_t value) {
    const uint32_t result = value << 5;
    const uint32_t previous = value << 4;
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | ((previous & 0x80000000u) != 0u ? 1u : 0u);
}

struct Unknown_00424fab_Record {
    uint8_t Unknown00[0x58];
    int16_t Unknown58;
    int16_t Unknown5A;
    int16_t Unknown5C;
};

// Builds the rotation block at 0x00622dc8 from the record in esi (0x00401f8e), then rotates the
// three words +0x58/+0x5a/+0x5c of the record (sign-extended, shifted left by 5, via
// DAT_00623e18/1c/20) with 0x00401f02 and rounds the result (FISTP) into DAT_00623e30/34/38.
// Leaves edx = 0x00622dc8, eax = the last shifted value and the flags of that shift.
// FUNCTION: GP4 0x00424fab
void __cdecl FUN_00424fab(gp4::Regs* r) {
    const Unknown_00424fab_Record* record =
        reinterpret_cast<const Unknown_00424fab_Record*>(static_cast<uintptr_t>(r->esi));
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622dc8);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401f8e, call);

    const uint32_t first = static_cast<uint32_t>(static_cast<int32_t>(record->Unknown58)) << 5;
    GP4_GLOBAL(uint32_t, 0x00623e18) = first;
    const uint32_t second = static_cast<uint32_t>(static_cast<int32_t>(record->Unknown5A)) << 5;
    GP4_GLOBAL(uint32_t, 0x00623e1c) = second;
    const int32_t thirdValue = static_cast<int32_t>(record->Unknown5C);
    const uint32_t third = static_cast<uint32_t>(thirdValue) << 5;
    GP4_GLOBAL(uint32_t, 0x00623e20) = third;

    GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e18));
    GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e1c));
    GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e20));

    call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401f02, call);

    GP4_GLOBAL(int32_t, 0x00623e30) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa0));
    GP4_GLOBAL(int32_t, 0x00623e34) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa8));
    GP4_GLOBAL(int32_t, 0x00623e38) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb0));

    r->eax = third;
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    r->eflags = (r->eflags & ~0x8d5u) | shl5Flags(static_cast<uint32_t>(thirdValue));
}
GP4_IMPL(0x00424fab, FUN_00424fab, "regs(esi:ptr[0x60]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

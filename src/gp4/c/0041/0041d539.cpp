#include <gp4/gp4.h>

struct Unknown_0041d539_Record {
    uint8_t Unknown00[0xac];
    int32_t UnknownAC;
    uint8_t UnknownB0[0x154 - 0xb0];
    int32_t Unknown154;
    int32_t Unknown158;
    int32_t Unknown15C;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of neg value (= cmp 0, value)
static inline uint32_t negFlags(uint32_t value) {
    const uint32_t result = 0u - value;
    return parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((value & result & 0x80000000u) != 0u ? 0x800u : 0u)
        | (value != 0u ? 1u : 0u);
}

// Takes the integers DAT_00623eb8/ebc/ec0 into DAT_00622be8/bec/bf0, runs the body block at
// 0x00622cf8 through 0x00402290 (integers -> doubles at +0xb8..), 0x00401f02 (rotation into
// +0xa0..) and 0x004022b5 (rounds +0xa0.. back to the integers DAT_00622bdc/be0/be4), stores
// those three into the record at esi+0x154/0x158/0x15c and the negated middle one into esi+0xac
// and DAT_0063803c. Leaves edx = 0x00622cf8, eax = -(esi+0x158) and the flags of that neg.
// FUNCTION: GP4 0x0041d539
void __cdecl FUN_0041d539(gp4::Regs* r) {
    Unknown_0041d539_Record* record =
        reinterpret_cast<Unknown_0041d539_Record*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00622be8) = GP4_GLOBAL(int32_t, 0x00623eb8);
    GP4_GLOBAL(int32_t, 0x00622bec) = GP4_GLOBAL(int32_t, 0x00623ebc);
    GP4_GLOBAL(int32_t, 0x00622bf0) = GP4_GLOBAL(int32_t, 0x00623ec0);

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00622cf8)));
    gp4::call_regs(0x00402290, *r);
    gp4::call_regs(0x00401f02, *r);
    gp4::call_regs(0x004022b5, *r);

    record->Unknown154 = GP4_GLOBAL(int32_t, 0x00622bdc);
    record->Unknown158 = GP4_GLOBAL(int32_t, 0x00622be0);
    record->Unknown15C = GP4_GLOBAL(int32_t, 0x00622be4);

    const uint32_t middle = static_cast<uint32_t>(record->Unknown158);
    const uint32_t negated = 0u - middle;
    record->UnknownAC = static_cast<int32_t>(negated);
    GP4_GLOBAL(uint32_t, 0x0063803c) = negated;

    r->eax = negated;
    r->eflags = (r->eflags & ~0x8d5u) | negFlags(middle);
}
GP4_IMPL(0x0041d539, FUN_0041d539, "regs(esi:ptr[0x168]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

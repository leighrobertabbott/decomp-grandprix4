#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_006380d8 {
    uint8_t Unknown00[0x4c];
    int32_t Unknown4C;
    int32_t Unknown50;
    int32_t Unknown54;
    uint8_t Unknown58[0x24c - 0x58];
    int32_t Unknown24C;
    int32_t Unknown250;
    int32_t Unknown254;
    uint8_t Unknown258[0x300 - 0x258];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record size");

// For each of the four 0x300-byte records at 0x006380d8: the dwords +0x4c/+0x50/+0x54 shifted left
// by 4 go through DAT_00623e18/1c/20 into doubles at +0xb8/+0xc0/+0xc8 of the body at 0x00622e98,
// 0x00401f02 rotates them (results at +0xa0/+0xa8/+0xb0), which are rounded (FISTP) back into the
// record at +0x24c/+0x250/+0x254. Then (DAT_00638044 << 4) + record.+0x250 - esi[k + 0x38c] is
// stored at esi[k + 0x2fc] (k = 4 * record index; esi is the caller's pointer and is restored).
// Leaves ebx = 0x006380d8, edx = the body, eax = the last stored value and the flags of the final
// "cmp ebx, 0x638cd8" (equal: ZF and PF set).
// FUNCTION: GP4 0x0041f1de
void __cdecl FUN_0041f1de(gp4::Regs* r) {
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);
    uint32_t* store = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->esi + 0x2fcu));
    const uint32_t* subtrahend = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi + 0x38cu));
    Unknown_006380d8* record = GP4_ARRAY(Unknown_006380d8, 0x006380d8);

    uint32_t value = 0;
    do {
        GP4_GLOBAL(uint32_t, 0x00623e18) = static_cast<uint32_t>(record->Unknown4C) << 4;
        GP4_GLOBAL(uint32_t, 0x00623e1c) = static_cast<uint32_t>(record->Unknown50) << 4;
        GP4_GLOBAL(uint32_t, 0x00623e20) = static_cast<uint32_t>(record->Unknown54) << 4;

        GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e18));
        GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e1c));
        GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x00623e20));

        gp4::Regs call = *r;
        call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
        gp4::call_regs(0x00401f02, call);

        record->Unknown24C = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa0));
        record->Unknown250 = gp4::x87::fistp32(GP4_FIELD(double, body, 0xa8));
        record->Unknown254 = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb0));

        value = (GP4_GLOBAL(uint32_t, 0x00638044) << 4) + static_cast<uint32_t>(record->Unknown250) - *subtrahend;
        *store = value;
        ++subtrahend;
        ++store;
        ++record;
    } while (reinterpret_cast<uintptr_t>(record) != 0x00638cd8u);

    r->eax = value;
    r->ebx = 0x006380d8u;
    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    // cmp ebx, 0x638cd8 with equal operands: ZF and PF set, everything else clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041f1de, FUN_0041f1de, "regs(esi:ptr[0x3a8]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

struct Unknown_0041d4e5_Record {
    uint8_t Unknown00[0x154];
    int32_t Unknown154;
    int32_t Unknown158;
    int32_t Unknown15C;
};

// FUNCTION: GP4 0x0041d4e5
// Copies the three dwords at esi+0x154/0x158/0x15c to DAT_00622bdc/be0/be4, then runs the body
// block at 0x00622cf8 through 0x00402246 (integers -> doubles at +0xa0..), 0x00401e2e (matrix
// multiply into +0xb8..) and 0x0040226b (rounds +0xb8.. back to the integers DAT_00622be8/bec/bf0)
// and copies those three results to DAT_00623eb8/ebc/ec0. Leaves edx = 0x00622cf8 and eax = the
// last copied value; the x87 stack is empty.
void __cdecl FUN_0041d4e5(gp4::Regs* r) {
    const Unknown_0041d4e5_Record* record =
        reinterpret_cast<const Unknown_0041d4e5_Record*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(int32_t, 0x00622bdc) = record->Unknown154;
    GP4_GLOBAL(int32_t, 0x00622be0) = record->Unknown158;
    GP4_GLOBAL(int32_t, 0x00622be4) = record->Unknown15C;

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00622cf8)));
    gp4::call_regs(0x00402246, *r);
    gp4::call_regs(0x00401e2e, *r);
    gp4::call_regs(0x0040226b, *r);

    GP4_GLOBAL(int32_t, 0x00623eb8) = GP4_GLOBAL(int32_t, 0x00622be8);
    GP4_GLOBAL(int32_t, 0x00623ebc) = GP4_GLOBAL(int32_t, 0x00622bec);
    const int32_t last = GP4_GLOBAL(int32_t, 0x00622bf0);
    GP4_GLOBAL(int32_t, 0x00623ec0) = last;
    r->eax = static_cast<uint32_t>(last);
}
GP4_IMPL(0x0041d4e5, FUN_0041d4e5, "regs(esi:ptr[0x164]:bytes) -> (eax, edx); globals=fuzz:i32+layout=view_00609860")

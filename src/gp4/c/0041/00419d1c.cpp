#include <gp4/gp4.h>

// One 0x300-byte record of the four at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown00[0x34];
    int32_t Unknown34;
    uint8_t Unknown38[4];
    int32_t Unknown3C;
    uint8_t Unknown40[0x80 - 0x40];
    uint32_t Unknown80;
    uint32_t Unknown84;
    uint32_t Unknown88;
    uint8_t Unknown8C[0x1f0 - 0x8c];
    int32_t Unknown1F0;
    uint8_t Unknown1F4[0x300 - 0x1f4];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// FUNCTION: GP4 0x00419d1c
// For each of the four records: feeds (Unknown34 + Unknown1F0, 0, Unknown3C) to the
// Q14 3x3 matrix transform 0x00401a33 (input at 0x00623e24.., result at 0x00623e18..)
// and stores the result vector at Unknown80/84/88.
// Leaves ebx at the end of the array, eax = the last z result, edx/ebp as the last
// call to 0x00401a33 left them, and the flags of the final cmp ebx (equal).
void __cdecl FUN_00419d1c(gp4::Regs* r) {
    Unknown_006380d8* rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    Unknown_006380d8* const end = GP4_ARRAY(Unknown_006380d8, 0x00638cd8);

    gp4::Regs callee = *r;
    do {
        GP4_GLOBAL(uint32_t, 0x00623e24) = static_cast<uint32_t>(rec->Unknown34) + static_cast<uint32_t>(rec->Unknown1F0);
        GP4_GLOBAL(uint32_t, 0x00623e28) = 0;
        GP4_GLOBAL(int32_t, 0x00623e2c) = rec->Unknown3C;
        gp4::call_regs(0x00401a33, callee);
        rec->Unknown80 = GP4_GLOBAL(uint32_t, 0x00623e18);
        rec->Unknown84 = GP4_GLOBAL(uint32_t, 0x00623e1c);
        rec->Unknown88 = GP4_GLOBAL(uint32_t, 0x00623e20);
        ++rec;
    } while (rec != end);

    r->eax = GP4_GLOBAL(uint32_t, 0x00623e20);
    r->ebx = reinterpret_cast<uintptr_t>(rec);
    r->edx = callee.edx;
    r->ebp = callee.ebp;
    // cmp ebx, 0x638cd8 with ebx == 0x638cd8: ZF | PF
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00419d1c, FUN_00419d1c, "regs() -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

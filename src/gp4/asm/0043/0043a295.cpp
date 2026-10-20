#include <gp4/gp4.h>
#include <gp4/x87.h>

// One sample: stores the displacement as doubles in the vector block, rotates it with the
// original matrix routine (0x00401f02), rounds the rotated vector into the three slots at
// 0x00608290.. and lets 0x0043a148 fold them into its tracked bounds.
static __declspec(noinline) void sample(gp4::Regs& regs, uint8_t* block, int32_t x, int32_t y, int32_t z) {
    GP4_FIELD(double, block, 0xb8) = (double)x;
    GP4_FIELD(double, block, 0xc0) = (double)y;
    GP4_FIELD(double, block, 0xc8) = (double)z;

    regs.edx = (uint32_t)(uintptr_t)block;
    gp4::call_regs(0x00401f02, regs);

    GP4_GLOBAL(int32_t, 0x00608290) = gp4::x87::fistp32(GP4_FIELD(double, block, 0xa0));
    GP4_GLOBAL(int32_t, 0x00608294) = gp4::x87::fistp32(GP4_FIELD(double, block, 0xa8));
    GP4_GLOBAL(int32_t, 0x00608298) = gp4::x87::fistp32(GP4_FIELD(double, block, 0xb0));

    gp4::call_regs(0x0043a148, regs);
}

// Runs eight samples: for each 16-byte record at edx, (edi - ebx + record) per axis is rotated
// and fed to the bounds tracker. pushal/popal around it restores every register; the flags left
// are those of the final "cmp ecx, 8" (equal).
// FUNCTION: GP4 0x0043a295
void __cdecl FUN_0043a295(gp4::Regs* r) {
    const int32_t* to = reinterpret_cast<const int32_t*>(r->edi);
    const int32_t* from = reinterpret_cast<const int32_t*>(r->ebx);
    const int32_t* offsets = reinterpret_cast<const int32_t*>(r->edx);
    uint8_t* block = GP4_ARRAY(uint8_t, 0x006232a8);

    gp4::Regs regs = *r;
    for (uint32_t i = 0; i < 8; i++) {
        const int32_t x = (int32_t)((uint32_t)to[0] - (uint32_t)from[0] + (uint32_t)offsets[0]);
        GP4_GLOBAL(int32_t, 0x00623e18) = x;
        const int32_t y = (int32_t)((uint32_t)to[1] - (uint32_t)from[1] + (uint32_t)offsets[1]);
        GP4_GLOBAL(int32_t, 0x00623e1c) = y;
        const int32_t z = (int32_t)((uint32_t)to[2] - (uint32_t)from[2] + (uint32_t)offsets[2]);
        GP4_GLOBAL(int32_t, 0x00623e20) = z;
        sample(regs, block, x, y, z);
        offsets += 4;
    }

    // popal restores the registers but not the flags: those of cmp ecx, 8 with ecx == 8
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043a295, FUN_0043a295, "regs(ebx:ptr[0x10]:bytes, edi:ptr[0x10]:bytes, edx:ptr[0x80]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bounds_0043a295")

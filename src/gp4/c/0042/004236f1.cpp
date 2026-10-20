#include <gp4/gp4.h>

// Clears the per-frame view accumulators (0x00608168..0x00608194, 0x00608274, 0x00608278,
// 0x0060828c), runs 0x0042320a unless the byte 0x0060827f is set, then the view-record pass
// 0x004233dd. Registers flow through both routines (push esi / pop esi keeps esi); the flags
// and registers left by 0x004233dd are the result.
// FUNCTION: GP4 0x004236f1
void __cdecl FUN_004236f1(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00608278) = 0;
    GP4_GLOBAL(uint32_t, 0x00608274) = 0;
    GP4_GLOBAL(uint32_t, 0x0060828c) = 0;
    GP4_GLOBAL(uint32_t, 0x00608168) = 0;
    GP4_GLOBAL(uint32_t, 0x0060816c) = 0;
    GP4_GLOBAL(uint32_t, 0x00608170) = 0;
    GP4_GLOBAL(uint32_t, 0x00608174) = 0;
    GP4_GLOBAL(uint32_t, 0x00608178) = 0;
    GP4_GLOBAL(uint32_t, 0x0060817c) = 0;
    GP4_GLOBAL(uint32_t, 0x00608180) = 0;
    GP4_GLOBAL(uint32_t, 0x00608184) = 0;
    GP4_GLOBAL(uint32_t, 0x00608188) = 0;
    GP4_GLOBAL(uint32_t, 0x0060818c) = 0;
    GP4_GLOBAL(uint32_t, 0x00608190) = 0;
    GP4_GLOBAL(uint32_t, 0x00608194) = 0;

    gp4::Regs c = *r;
    if (GP4_GLOBAL(uint8_t, 0x0060827f) == 0) {
        gp4::call_regs(0x0042320a, c);
    }
    gp4::call_regs(0x004233dd, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x004236f1, FUN_004236f1, "regs(edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+state_00424011+state_00421fc3+state_0042320a")

#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00402655
// Builds a 3x... rotation sub-matrix entry set from three Euler angles. eax/ebx/ecx hold the
// angles as binary angles (2^31 = pi); each is halved (sar 1), stored to 0x00622bd4/bd0/bd8
// and scaled by pi/2^31 (0x006f2bdc) into the doubles 0x00622c98/ca0/ca8. sin/cos of those are
// stored at 0x00622cc8..cf0 and the four products below are written at edx + 0x00622c00..c18
// (edx = byte offset of the destination slot, 0x20 or 0x40 at the call sites). Products and
// sums are kept on the x87 stack at extended precision and rounded at each qword store.
void __cdecl FUN_00402655(gp4::Regs* r) {
    using namespace gp4::x87;

    const uint32_t offset = r->edx;
    const int32_t a = (int32_t)r->eax >> 1;
    const int32_t b = (int32_t)r->ebx >> 1;
    const int32_t c = (int32_t)r->ecx >> 1;

    GP4_GLOBAL(int32_t, 0x00622bd4) = a;
    GP4_GLOBAL(int32_t, 0x00622bd0) = b;
    GP4_GLOBAL(int32_t, 0x00622bd8) = c;

    GP4_GLOBAL(double, 0x00622c98) = narrow(ext_i32(GP4_GLOBAL(int32_t, 0x00622bd0)) * ext(GP4_GLOBAL(double, 0x006f2bdc)));
    GP4_GLOBAL(double, 0x00622ca0) = narrow(ext_i32(GP4_GLOBAL(int32_t, 0x00622bd4)) * ext(GP4_GLOBAL(double, 0x006f2bdc)));
    GP4_GLOBAL(double, 0x00622ca8) = narrow(ext_i32(GP4_GLOBAL(int32_t, 0x00622bd8)) * ext(GP4_GLOBAL(double, 0x006f2bdc)));

    fsincos(GP4_GLOBAL(double, 0x00622c98), &GP4_GLOBAL(double, 0x00622cc8), &GP4_GLOBAL(double, 0x00622cd0));
    fsincos(GP4_GLOBAL(double, 0x00622ca0), &GP4_GLOBAL(double, 0x00622cd8), &GP4_GLOBAL(double, 0x00622ce0));
    fsincos(GP4_GLOBAL(double, 0x00622ca8), &GP4_GLOBAL(double, 0x00622ce8), &GP4_GLOBAL(double, 0x00622cf0));

    const Ext sin1 = ext(GP4_GLOBAL(double, 0x00622cc8));
    const Ext cos1 = ext(GP4_GLOBAL(double, 0x00622cd0));
    const Ext sin2 = ext(GP4_GLOBAL(double, 0x00622cd8));
    const Ext cos2 = ext(GP4_GLOBAL(double, 0x00622ce0));
    const Ext sin3 = ext(GP4_GLOBAL(double, 0x00622ce8));
    const Ext cos3 = ext(GP4_GLOBAL(double, 0x00622cf0));

    uint8_t* const base = (uint8_t*)0x00622c00 + offset;

    const Ext p = cos2 * sin1;
    const Ext q = sin2 * cos1;
    *(double*)(base + 0x08) = narrow(cos3 * p + sin3 * q);
    *(double*)(base + 0x10) = narrow(sin3 * p - cos3 * q);

    const Ext s = cos2 * cos1;
    const Ext t = sin2 * sin1;
    *(double*)(base + 0x18) = narrow(sin3 * s - cos3 * t);
    *(double*)(base + 0x00) = narrow(cos3 * s + sin3 * t);

    // The last flag writer is "sar ecx, 1".
    uint32_t par = (uint32_t)c & 0xffu;
    par ^= par >> 4;
    par ^= par >> 2;
    par ^= par >> 1;
    uint32_t fl = r->eflags & ~(0x1u | 0x4u | 0x10u | 0x40u | 0x80u | 0x800u);
    fl |= (r->ecx & 1u);
    fl |= (par & 1u) ? 0u : 0x4u;
    fl |= (c == 0) ? 0x40u : 0u;
    fl |= ((uint32_t)c & 0x80000000u) ? 0x80u : 0u;

    r->eax = (uint32_t)a;
    r->ebx = (uint32_t)b;
    r->ecx = (uint32_t)c;
    r->eflags = fl;
}
GP4_IMPL(0x00402655, FUN_00402655, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32[0..0x40]) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of)")

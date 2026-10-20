#include <gp4/gp4.h>
#include <gp4/flags.h>

// Rotates the position vector of the record ebx by its three angles and re-bases two coordinates.
// The record's angle words +0xc2 / +0xc6 / +0xca go to 0x7a8de2 / 0x7a8de0 / 0x7a8de4 and
// 0x00551880 turns them into Q14 sines and cosines; 0x0051e841 builds the 3x3 rotation matrix from them;
// the vector (+0x78, +0x7c, +0x80) goes to 0x8754b8..0x8754c0 and 0x0051eb1c multiplies it through the
// matrix into 0x8754ac / 0x8754b0 / 0x8754b4. The caller's coordinates 0x934df0 and 0x934df4 and the
// register ecx are then reduced by the first, third and second rotated component respectively:
// [0x934df0] -= [0x8754ac], ecx -= [0x8754b0], [0x934df4] -= [0x8754b4]. eax = [0x8754b4]; edx and ebp
// are the 0x0051eb1c results; the flags are those of the final subtraction in memory.
// FUNCTION: GP4 0x00552c61
void __cdecl FUN_00552c61(gp4::Regs* r) {
    const uint32_t record = r->ebx;

    GP4_GLOBAL(uint16_t, 0x007a8de2) = GP4_FIELD(uint16_t, record, 0xc2);
    GP4_GLOBAL(uint16_t, 0x007a8de0) = GP4_FIELD(uint16_t, record, 0xc6);
    GP4_GLOBAL(uint16_t, 0x007a8de4) = GP4_FIELD(uint16_t, record, 0xca);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00551880, c);
    gp4::call_regs(0x0051e841, c);

    GP4_GLOBAL(uint32_t, 0x008754b8) = GP4_FIELD(uint32_t, record, 0x78);
    GP4_GLOBAL(uint32_t, 0x008754bc) = GP4_FIELD(uint32_t, record, 0x7c);
    GP4_GLOBAL(uint32_t, 0x008754c0) = GP4_FIELD(uint32_t, record, 0x80);
    gp4::call_regs(0x0051eb1c, c);

    GP4_GLOBAL(uint32_t, 0x00934df0) -= GP4_GLOBAL(uint32_t, 0x008754ac);
    const uint32_t y = GP4_GLOBAL(uint32_t, 0x008754b0);
    r->ecx -= y;
    const uint32_t z = GP4_GLOBAL(uint32_t, 0x008754b4);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00934df4);
    GP4_GLOBAL(uint32_t, 0x00934df4) = before - z;

    r->eax = z;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(before, z));
}
GP4_IMPL(0x00552c61, FUN_00552c61, "regs(ebx:ptr[0xd2]:bytes, ecx:u32) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

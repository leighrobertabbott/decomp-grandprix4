#include <gp4/gp4.h>
#include <gp4/flags.h>

// Angular distance between a heading and the reference angle [0x679b90] (a 16-bit angle, 0x8000 =
// half turn). The word at edi and then the word at the record [0x679d28] (only when it lies inside
// the ring [0x891500, 0x866ef8)) are reduced the same way: d = word - ref, |d|; when |d| >= 0x4000
// the distance becomes 0x8000 - |d| and the "behind" marker 0x8000 is set (cx). The first result
// goes to the words 0x875e92 (marker) and 0x875e96 (distance), the second one (or, for a record
// outside the ring, the unchanged first result still in cx/ax) to 0x875e90 and 0x875e94; the
// dwords 0x6f3ad0/0x6f3ad4 are cleared. Only ax/cx and edx change; the flags are those of the last
// executed cmp/add/neg.
// FUNCTION: GP4 0x00526f21
void __cdecl FUN_00526f21(gp4::Regs* r) {
    uint32_t arith = 0;

    uint16_t cx = 0;
    uint16_t ax = GP4_FIELD(uint16_t, r->edi, 0);
    {
        const uint16_t ref = GP4_GLOBAL(uint16_t, 0x00679b90);
        arith = gp4::flags::sub16(ax, ref);
        ax = static_cast<uint16_t>(ax - ref);
        if ((arith & gp4::flags::SF) != 0u) {
            arith = gp4::flags::neg16(ax);
            ax = static_cast<uint16_t>(0u - ax);
        }
        arith = gp4::flags::sub16(ax, 0x4000u);
        if (ax >= 0x4000u) {
            arith = gp4::flags::neg16(ax);
            ax = static_cast<uint16_t>(0u - ax);
            arith = gp4::flags::add16(ax, 0x8000u);
            ax = static_cast<uint16_t>(ax + 0x8000u);
            cx = 0x8000u;
        }
    }
    GP4_GLOBAL(uint16_t, 0x00875e92) = cx;
    GP4_GLOBAL(uint16_t, 0x00875e96) = ax;

    const uint32_t edx = GP4_GLOBAL(uint32_t, 0x00679d28);
    arith = gp4::flags::sub32(edx, GP4_GLOBAL(uint32_t, 0x00891500));
    if (edx >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        arith = gp4::flags::sub32(edx, GP4_GLOBAL(uint32_t, 0x00866ef8));
        if (edx < GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            cx = 0;
            ax = GP4_FIELD(uint16_t, edx, 0);
            const uint16_t ref = GP4_GLOBAL(uint16_t, 0x00679b90);
            arith = gp4::flags::sub16(ax, ref);
            ax = static_cast<uint16_t>(ax - ref);
            if ((arith & gp4::flags::SF) != 0u) {
                arith = gp4::flags::neg16(ax);
                ax = static_cast<uint16_t>(0u - ax);
            }
            arith = gp4::flags::sub16(ax, 0x4000u);
            if (ax >= 0x4000u) {
                arith = gp4::flags::neg16(ax);
                ax = static_cast<uint16_t>(0u - ax);
                arith = gp4::flags::add16(ax, 0x8000u);
                ax = static_cast<uint16_t>(ax + 0x8000u);
                cx = 0x8000u;
            }
        }
    }
    GP4_GLOBAL(uint16_t, 0x00875e90) = cx;
    GP4_GLOBAL(uint16_t, 0x00875e94) = ax;
    GP4_GLOBAL(uint32_t, 0x006f3ad0) = 0;
    GP4_GLOBAL(uint32_t, 0x006f3ad4) = 0;

    r->eax = (r->eax & 0xffff0000u) | ax;
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00526f21, FUN_00526f21, "regs(edi:ptr[0x40]:bytes, eax:u32, ecx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00555b28+ring2_00526f21")

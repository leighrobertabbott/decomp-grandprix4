#include <gp4/gp4.h>
#include <gp4/flags.h>

// Latches the previous values of the two bytes/words at 0x8705be/0x8705bc, then derives two
// per-frame deltas: the 8.8 fixed-point difference between the targets (0x8705ad / 0x8705ac) and
// the current values (0x8705f9 / 0x8705f8, stored shifted by 8 at 0x8705ae / 0x8705b0), divided
// by the frame counter word 0x934dfc (1 when zero) and stored at 0x8705b2 / 0x8705b4.
// Leaves eax = the last quotient (or 0), ebx = the last current value << 8, edx/ebp from the
// last executed cdq/idiv/movzx, and the flags of the last instruction that wrote them.
// FUNCTION: GP4 0x0055a593
void __cdecl FUN_0055a593(gp4::Regs* r) {
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    GP4_GLOBAL(uint8_t, 0x008705c0) = 0;
    GP4_GLOBAL(uint8_t, 0x0087062e) = GP4_GLOBAL(uint8_t, 0x008705be);
    GP4_GLOBAL(uint16_t, 0x0087062c) = GP4_GLOBAL(uint16_t, 0x008705bc);
    GP4_GLOBAL(uint8_t, 0x008705be) = static_cast<uint8_t>(GP4_GLOBAL(uint16_t, 0x008705ec));

    uint32_t ebx = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x008705f9)) << 8;
    GP4_GLOBAL(uint16_t, 0x008705ae) = static_cast<uint16_t>(ebx);
    uint32_t eax = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x008705ad)) << 8;
    uint32_t arith = gp4::flags::sub32(eax, ebx);
    eax -= ebx;
    if (eax != 0u) {
        edx = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 31);
        ebp = GP4_GLOBAL(uint16_t, 0x00934dfc);
        arith = gp4::flags::logic32(ebp);
        if (ebp == 0u) {
            ebp = 1u;
        }
        int32_t rem;
        eax = static_cast<uint32_t>(gp4::idiv64(static_cast<int32_t>(eax), static_cast<int32_t>(ebp), &rem));
        edx = static_cast<uint32_t>(rem);
    }
    GP4_GLOBAL(uint16_t, 0x008705b2) = static_cast<uint16_t>(eax);

    ebx = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x008705f8)) << 8;
    GP4_GLOBAL(uint16_t, 0x008705b0) = static_cast<uint16_t>(ebx);
    eax = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x008705ac)) << 8;
    arith = gp4::flags::sub32(eax, ebx);
    eax -= ebx;
    if (eax != 0u) {
        edx = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 31);
        ebp = GP4_GLOBAL(uint16_t, 0x00934dfc);
        arith = gp4::flags::logic32(ebp);
        if (ebp == 0u) {
            ebp = 1u;
        }
        int32_t rem;
        eax = static_cast<uint32_t>(gp4::idiv64(static_cast<int32_t>(eax), static_cast<int32_t>(ebp), &rem));
        edx = static_cast<uint32_t>(rem);
    }
    GP4_GLOBAL(uint16_t, 0x008705b4) = static_cast<uint16_t>(eax);

    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055a593, FUN_0055a593, "regs(edx:u32, ebp:u32) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

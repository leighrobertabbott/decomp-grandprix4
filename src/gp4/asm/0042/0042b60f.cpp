#include <gp4/gp4.h>

// Advances a track-segment pointer by [0x603e9c] with the ring wraps of the 0x160-byte segment array:
// below the pit start [0x891500] a result at or above the lap end [0x866ecc] wraps by [0x866ec4];
// from the pit start on a result at or above the pit end [0x866ef8] becomes pit end - 0x160.
static inline uint32_t advanceSegment(uint32_t segment) {
    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        segment += GP4_GLOBAL(uint32_t, 0x00603e9c);
        if (segment >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            segment -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else {
        segment += GP4_GLOBAL(uint32_t, 0x00603e9c);
        if (segment >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            segment = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return segment;
}

// FUNCTION: GP4 0x0042b60f
// esi = car A, ebx = car B. The word at +0x16 of A's segment ([esi+0x1b8], advanced by [0x603e9c]) goes
// to 0x006077c7; 0x004035c3 interpolates with the blend factor movsx [esi+0x66] on that segment (its
// result goes to 0x00604402). Then ebx and esi are exchanged and B's segment ([ebx+0x1c], advanced the
// same way) is the transform for 0x0040404c, which rotates B's position words (+0x90, +0x92); 0x00403d57
// then rotates the result using A's segment. Leaves ecx = (interpolation eax high : the rotated x word
// [0x934df0]), eax = (rotation eax high : [0x604402]); ebx and esi swapped; edi restored.
void __cdecl FUN_0042b60f(gp4::Regs* r) {
    uint8_t* carA = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t segmentA = advanceSegment(GP4_FIELD(uint32_t, carA, 0x1b8));
    GP4_GLOBAL(uint16_t, 0x006077c7) =
        GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segmentA)), 0x16);

    gp4::Regs interp = *r;
    interp.st_in = 0;
    interp.st_out = 0;
    interp.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, carA, 0x66))));
    interp.edi = segmentA;
    gp4::call_regs(0x004035c3, interp);
    GP4_GLOBAL(uint16_t, 0x00604402) = static_cast<uint16_t>(interp.eax);

    // xchg ebx, esi
    const uint32_t carBPtr = interp.ebx;
    const uint32_t carAPtr = interp.esi;
    uint8_t* carB = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(carBPtr));
    const uint32_t segmentB = advanceSegment(GP4_FIELD(uint32_t, carB, 0x1c));

    gp4::Regs rotate = interp;
    rotate.ebx = carAPtr;
    rotate.esi = carBPtr;
    rotate.edi = segmentB;
    gp4::call_regs(0x0040404c, rotate);

    rotate.edi = segmentA;
    gp4::call_regs(0x00403d57, rotate);

    r->eax = (rotate.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00604402);
    r->ecx = (interp.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00934df0);
    r->edx = rotate.edx;
    r->ebx = carAPtr;
    r->esi = carBPtr;
    r->ebp = interp.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (rotate.eflags & 0x8d5u);
}
GP4_IMPL(0x0042b60f, FUN_0042b60f, "regs(ebx:ptr[0x238]:layout=car_0043c677, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+car_0042b17e) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+step_0042b17e+geom_008943a0")

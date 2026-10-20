#include <gp4/gp4.h>

// FUNCTION: GP4 0x0042b6c2
// Rotates the position pair at esi around a track segment record: edi starts as the segment pointer
// [esi+0x1c], is stepped along its link by 0x00406123 (CF set = no link: take [ebx+0x1b8] instead), then
// advanced by [0x603e9c] with the ring wraps of the 0x160-byte segment array (lap end [0x866ecc] minus
// [0x866ec4] below the pit start [0x891500], else pit end [0x866ef8] - 0x160). The word at +0x16 of
// the record that segment's link dword (+0x44) points at (0 when none) goes to 0x006077c7, and the two
// position dwords are rotated through 0x00403d57 using that segment record. Leaves ecx = (ecx after
// 0x00406123 high : the rotated x word [0x934df0]), eax = (rotation eax high : word [ebx+0x90]);
// edx and the flags are those of 0x00403d57; edi is restored.
void __cdecl FUN_0042b6c2(gp4::Regs* r) {
    uint8_t* point = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    gp4::Regs link = *r;
    link.edi = GP4_FIELD(uint32_t, point, 0x1c);
    link.st_in = 0;
    link.st_out = 0;
    gp4::call_regs(0x00406123, link);

    uint32_t segment = link.edi;
    if ((link.eflags & 1u) != 0u) {
        segment = GP4_FIELD(uint32_t, car, 0x1b8);
    }

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

    uint32_t eax = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment)), 0x44);
    if (eax != 0u) {
        eax = (eax & 0xffff0000u) | GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(eax)), 0x16);
    }
    GP4_GLOBAL(uint16_t, 0x006077c7) = static_cast<uint16_t>(eax);

    GP4_GLOBAL(uint32_t, 0x00934df0) = GP4_FIELD(uint32_t, point, 0);
    GP4_GLOBAL(uint32_t, 0x00934df4) = GP4_FIELD(uint32_t, point, 4);

    gp4::Regs rotate = link;
    rotate.eax = GP4_FIELD(uint32_t, point, 4);
    rotate.edi = segment;
    rotate.st_in = 0;
    rotate.st_out = 0;
    gp4::call_regs(0x00403d57, rotate);

    r->eax = (rotate.eax & 0xffff0000u) | GP4_FIELD(uint16_t, car, 0x90);
    r->ecx = (link.ecx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00934df0);
    r->edx = rotate.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (rotate.eflags & 0x8d5u);
}
GP4_IMPL(0x0042b6c2, FUN_0042b6c2, "regs(eax:u32, ecx:u32, edx:u32, ebx:ptr[0x1c0]:layout=other_0042b6c2, esi:ptr[0x238]:layout=car_0043c677+pos_00405cf8) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+links_00405a12+ahead_0042b6c2")

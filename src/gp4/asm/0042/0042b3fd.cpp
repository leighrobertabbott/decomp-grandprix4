#include <gp4/gp4.h>

// Track-segment record (edi): the two lane words read here.
struct Unknown_0042b3fd_Segment {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
};

// FUNCTION: GP4 0x0042b3fd
// esi = point (two dwords), edi = segment record. When edi lies below the pit boundary [0x00891500]
// 0x0042b3b8 is tried first (it moves edi on to a linked record); CF = 0 from it returns CF = 0
// at once with its eax / ecx / edx. Otherwise the point is copied to [0x934df0]/[0x934df4] and
// rotated into the segment's frame by 0x00403d57. Returns eax = the rotated x word (sign-extended),
// edx = the segment's lane word (+0x60 when byte [0x868582] is negative, else +0x62) + 0x50,
// ecx = the (possibly moved) segment, CF = 1 and the other flags those of 0x00403d57. ebx, ebp, esi
// and edi are restored.
void __cdecl FUN_0042b3fd(gp4::Regs* r) {
    const uint32_t* point = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t segment = r->edi;
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;

    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        gp4::Regs link = *r;
        link.st_in = 0;
        link.st_out = 0;
        gp4::call_regs(0x0042b3c8, link);
        eax = link.eax;
        ecx = link.ecx;
        edx = link.edx;
        segment = link.edi;
        if ((link.eflags & 1u) == 0u) {
            r->eax = eax;
            r->ecx = ecx;
            r->edx = edx;
            r->eflags = (r->eflags & ~0x8d5u) | (link.eflags & 0x8d4u);  // clc
            return;
        }
    }

    const Unknown_0042b3fd_Segment* record = reinterpret_cast<const Unknown_0042b3fd_Segment*>(static_cast<uintptr_t>(segment));
    const uint32_t lane = (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0) ? record->Unknown60 : record->Unknown62;
    edx = lane + 0x50u;

    GP4_GLOBAL(uint32_t, 0x00934df0) = point[0];
    eax = point[1];
    GP4_GLOBAL(uint32_t, 0x00934df4) = eax;

    gp4::Regs rotate = *r;
    rotate.st_in = 0;
    rotate.st_out = 0;
    rotate.eax = eax;
    rotate.edx = edx;
    rotate.edi = segment;
    gp4::call_regs(0x00403d57, rotate);

    r->eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00934df0)));
    r->edx = edx;
    r->ecx = segment;
    r->eflags = (r->eflags & ~0x8d5u) | (rotate.eflags & 0x8d5u) | 1u;  // stc
}
GP4_IMPL(0x0042b3fd, FUN_0042b3fd, "regs(esi:ptr[0x40]:layout=pos_00405cf8, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00405a12+links_0042b3c8+segw_0042b3fd")

#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043ac7f
// Collision chain for the object pair ebx / edi. 0x0043aa07 decides (CF set) whether the pair needs
// work; then, with esi = edi, 0x0043aac6 and 0x00438c98 run while the position triple [ebx..ebx+8]
// is saved and restored around them. When 0x00438c98 leaves CF set the pair is ordered
// (0x00438c1f), resolved (0x0043a83d) and the cars are reset (0x0043ad30). esi is restored.
void __cdecl FUN_0043ac7f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0043aa07, c);
    if ((c.eflags & 1u) != 0u) {
        const uint32_t savedEsi = c.esi;
        c.esi = c.edi;

        const uint32_t* position = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.ebx));
        const uint32_t x = position[0];
        const uint32_t y = position[1];
        const uint32_t z = position[2];

        gp4::call_regs(0x0043aac6, c);
        gp4::call_regs(0x00438c98, c);

        uint32_t* restore = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.ebx));
        restore[2] = z;
        restore[1] = y;
        restore[0] = x;

        if ((c.eflags & 1u) != 0u) {
            gp4::call_regs(0x00438c1f, c);
            gp4::call_regs(0x0043a83d, c);
            gp4::call_regs(0x0043ad30, c);
        }
        c.esi = savedEsi;
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043ac7f, FUN_0043ac7f, "regs(ebx:ptr[0x78]:layout=obj_0043ac7f, edi:ptr[0x78]:layout=obj_0043ac7f) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bytes_0063bcfc+flags_00408f7b+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+segflag_004381ab+slots_0043ad30+carst_0040f6d0+slots_0043a05a+carcc_0043a785+shape_006099c0")

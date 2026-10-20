#include <gp4/gp4.h>
#include <gp4/flags.h>

// The message box record behind [0x0083e6a2]: a text id at +0 (0x0054fe80 stores it) and the box
// corners as words at +4 / +6 (x0, y0) and +8 / +0xa (x1, y1).
#pragma pack(push, 1)
struct MessageBox {
    uint32_t TextId00;
    uint16_t X0_04;
    uint16_t Y0_06;
    uint16_t X1_08;
    uint16_t Y1_0a;
};
#pragma pack(pop)

// Draws the message box: sets the colour 0x7fff (0x0054cc96), queues the box (corners moved 0x10 inwards) in
// the sorted list 0x00549db9, runs the empty routines 0x00531b3d / 0x0054d70a, stores ebx + 4 as the text
// cursor x (0x0054cbca) and prints the string of the box's text id (0x0054d1a2, esi = the id). Every
// register and the flags carry from one call into the next; ebx, eax, ebp, ecx and esi are left as set up
// for the list insert / text output, the flags are those 0x0054d1a2 leaves.
// FUNCTION: GP4 0x0054fee0
void __cdecl FUN_0054fee0(gp4::Regs* r) {
    r->eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, *r);

    const MessageBox* box = reinterpret_cast<const MessageBox*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0083e6a2)));
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(box));
    r->ebx = static_cast<uint32_t>(box->X0_04) + 0x10u;
    r->ebp = static_cast<uint32_t>(box->X1_08) - 0x10u;
    r->eax = static_cast<uint32_t>(box->Y0_06) + 0x10u;
    r->ecx = static_cast<uint32_t>(box->Y1_0a) - 0x10u;
    gp4::call_regs(0x00549db9, *r);

    gp4::cdecl_<0x00531b3d, void>();
    gp4::cdecl_<0x0054d70a, void>();

    const uint32_t x = r->ebx;
    r->ebx = x + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(x, 4u));
    gp4::call_regs(0x0054cbca, *r);

    r->esi = box->TextId00;
    gp4::call_regs(0x0054d1a2, *r);
}
GP4_IMPL(0x0054fee0, FUN_0054fee0, "regs(ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+script_0054d798+log_004fd730+textout_0054d211+tbox2_0054fee0")

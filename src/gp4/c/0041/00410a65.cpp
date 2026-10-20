#include <gp4/gp4.h>

// FUNCTION: GP4 0x00410a65
// pushal ... popal routine over the car record in esi. 0x0040fb09 selects the car's data block
// ([0x00601f24]); the block's offset from 0x007adb4c goes to dword +0x104 of the record and the
// block pointer becomes ebx for the setup chain (0x410a40, 0x40fbf3, 0x4102df, 0x41034e, 0x4104b1,
// 0x410515), then 0x410a1c runs for wheels ecx = 0..3 and 0x4108c0 / 0x4109aa finish it. Every
// register is restored by popal; the flags escape from the last callee (0x4109aa).
void __cdecl FUN_00410a65(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0040fb09, c);

    c.eax = GP4_GLOBAL(uint32_t, 0x00601f24) - 0x007adb4cu;
    GP4_FIELD(uint32_t, car, 0x104) = c.eax;
    c.ebx = GP4_GLOBAL(uint32_t, 0x00601f24);

    gp4::call_regs(0x00410a40, c);
    gp4::call_regs(0x0040fbf3, c);
    gp4::call_regs(0x004102df, c);
    gp4::call_regs(0x0041034e, c);
    gp4::call_regs(0x004104b1, c);
    gp4::call_regs(0x00410515, c);

    const uint32_t savedEbx = c.ebx;
    const uint32_t savedEsi = c.esi;
    c.ecx = 0;
    do {
        gp4::call_regs(0x00410a1c, c);
        c.ecx = c.ecx + 1u;
    } while (c.ecx != 4u);
    c.esi = savedEsi;
    c.ebx = savedEbx;

    gp4::call_regs(0x004108c0, c);
    gp4::call_regs(0x004109aa, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00410a65, FUN_00410a65, "regs(esi:ptr[0x420]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=setup_007adb54+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+wheel_00410a1c+state_0040fbf3+state_00410a65")

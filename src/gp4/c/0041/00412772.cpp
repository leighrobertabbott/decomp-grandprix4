#include <gp4/gp4.h>

// The four 0x300-byte wheel records at 0x006380d8 (ebx), reached by displacement.
static __declspec(noinline) void setWheelBytes(uint8_t* wheels) {
    GP4_FIELD(uint8_t, wheels, 0x2c) = 0x40;
    GP4_FIELD(uint8_t, wheels, 0x32c) = 0x40;
    GP4_FIELD(uint8_t, wheels, 0x62c) = 0x20;
    GP4_FIELD(uint8_t, wheels, 0x92c) = 0x20;
}

// FUNCTION: GP4 0x00412772
// Car setup: derives the physics constants (0x004112e3), clears the wheel/body state block
// (0x00411f73), builds the derived tables (0x004123c6) and the 0x00679b48 constant (0x00412408), clears
// the word 0x00638d40, sets byte +0x2c of the wheel records at 0x006380d8 to 0x40 / 0x40 / 0x20 / 0x20
// and finishes with the per-wheel hand-over 0x00416708. Registers and flags are whatever the callees
// leave (ebx = 0x006380d8 at the last call).
void __cdecl FUN_00412772(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x004112e3, c);
    gp4::call_regs(0x00411f73, c);
    gp4::call_regs(0x004123c6, c);
    gp4::call_regs(0x00412408, c);

    GP4_GLOBAL(uint16_t, 0x00638d40) = 0;
    c.ebx = 0x006380d8u;
    setWheelBytes(GP4_ARRAY(uint8_t, 0x006380d8));

    gp4::call_regs(0x00416708, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00412772, FUN_00412772, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004121c7+state_004120e4+state_00411f9f+state_004123a6+frac_00624790")

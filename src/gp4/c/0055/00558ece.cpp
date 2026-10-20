#include <gp4/gp4.h>

// FUNCTION: GP4 0x00558ece
// Runs the per-attribute check 0x00558e7b eight times for the record in edi (0x160-byte car/ring
// record): ebx = word offset 0xb..0xe, 0x14..0x17 into its 0x60-byte entry of the table at 0x9559f8,
// ebp = the matching attribute bit 0x800 .. 0x40000 of the dword at record+0x2c. ecx starts at 0 and
// accumulates the bits the callee adds; the routine returns that mask in eax (and ecx). ebx is
// pushed/popped; ebp is left holding the last bit (0x40000) and the flags are those the last
// callee run left.
void __cdecl FUN_00558ece(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.ecx = 0u;

    c.ebx = 0x0bu;
    c.ebp = 0x00000800u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x0cu;
    c.ebp = 0x00001000u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x0du;
    c.ebp = 0x00002000u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x0eu;
    c.ebp = 0x00004000u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x14u;
    c.ebp = 0x00008000u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x15u;
    c.ebp = 0x00010000u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x16u;
    c.ebp = 0x00020000u;
    gp4::call_regs(0x00558e7b, c);
    c.ebx = 0x17u;
    c.ebp = 0x00040000u;
    gp4::call_regs(0x00558e7b, c);

    r->eax = c.ecx;
    r->ecx = c.ecx;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00558ece, FUN_00558ece, "regs(edi:ptr[0x60]:layout=rec_00558ece) -> (eax, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0051cc50+attr_00558e7b")

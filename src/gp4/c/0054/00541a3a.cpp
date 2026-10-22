#include <gp4/gp4.h>
#include <gp4/flags.h>

// Frees the 22-slot table entry of the id in eax. The slot index of the id (0x00541d8d, -1 when
// there is none) selects the qword at 0x0083002b + 8 * index (unaligned), which receives the
// performance counter (0x0054e733) with 0x60 added to its high dword. Then the id is looked up again
// (0x00541d06) and the first dword of the slot found is cleared. eax is restored; edx and edi are
// left as the counter read leaves them, the flags are those of the final `cmp eax, 0` on the slot
// address (or 0).
// FUNCTION: GP4 0x00541a3a
void __cdecl FUN_00541a3a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x00541d8d, c);
    if (c.eax != 0xffffffffu) {
        c.edi = (c.eax << 3) + 0x0083002bu;
        gp4::call_regs(0x0054e733, c);
        c.edx += 0x60u;
        GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)), 0) = c.eax;
        GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)), 4) = c.edx;
    }

    c.eax = r->eax;
    gp4::call_regs(0x00541d06, c);
    const uint32_t slot = c.eax;
    if (slot != 0u) {
        GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(slot)), 0) = 0u;
    }

    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(slot, 0u));
}
GP4_IMPL(0x00541a3a, FUN_00541a3a, "regs(eax:u32{0,1,0x1f1f,0xffffffff,5}) -> (ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+qpc_0054e733")

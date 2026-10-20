#include <gp4/gp4.h>

// Picks the best candidate car of the queue (0x0042fa52: ebx = car, ebp = its key) and, unless the key is
// 0x6000, runs the pair check 0x0042cd1e between the car in esi and the candidate. When that check clears
// carry and the car's word +0x1bc holds the candidate's offset in the car array (0x66fef0 + word == ebx),
// the word is reset to 0xffff. 0x0042faa9 finishes in every case; registers and flags are whatever the
// callees leave.
// FUNCTION: GP4 0x0042faf5
void __cdecl FUN_0042faf5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0042fa52, c);
    if ((c.eflags & 1u) == 0u && c.ebp != 0x6000u) {
        gp4::call_regs(0x0042cd1e, c);
        if ((c.eflags & 1u) == 0u) {
            uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
            uint32_t eax = GP4_FIELD(uint16_t, car, 0x1bc);
            c.eax = eax;
            if (static_cast<uint16_t>(eax) != 0xffffu) {
                eax += 0x0066fef0u;
                c.eax = eax;
                if (c.ebx == eax) {
                    GP4_FIELD(uint16_t, car, 0x1bc) = 0xffffu;
                }
            }
        }
    }
    gp4::call_regs(0x0042faa9, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0042faf5, FUN_0042faf5, "regs(ebx:u32, esi:rec[0x66fef0,0x41c,0..21], ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+cand_0042faf5; maxinsn=400000")

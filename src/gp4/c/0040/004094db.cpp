#include <gp4/gp4.h>

// pushal; call 0x004094e3; popal; ret
// The callee gets the entry registers; popal then restores every general register to its entry
// value but leaves the flags of the callee's last flag writer. edi must cover the +0x176 word the
// callee reads through [esi+0x1b8] (= edi).
// FUNCTION: GP4 0x004094db
void __cdecl FUN_004094db(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;
    const uint32_t edx = r->edx;
    const uint32_t esi = r->esi;
    const uint32_t edi = r->edi;
    const uint32_t ebp = r->ebp;

    gp4::call_regs(0x004094e3, *r);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->ebp = ebp;
}
GP4_IMPL(0x004094db, FUN_004094db, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, edi:rec[0x8943a0,0x160,0..11], esi:ptr[0x420]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+state_004094e3")

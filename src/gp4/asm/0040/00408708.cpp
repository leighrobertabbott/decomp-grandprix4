#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408708
void __cdecl FUN_00408708(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    body[0x7a] |= 0x40;
    body[0x7b] &= 0x7f;
    body[0x25f] &= 0xbf;
    body[0x25f] &= 0x7f;
    body[0xfe] &= 0xfb;
    body[0x7a] |= 0x20;
    body[0xcc] |= 0x10;
    body[0xce] |= 0x10;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x004086f9, callee);
    gp4::call_regs(0x0043b9ae, callee);

    r->eax = callee.eax;
    r->ebx = callee.ebx;
    r->ecx = callee.ecx;
    r->edx = callee.edx;
    r->esi = callee.esi;
    r->edi = callee.edi;
    r->ebp = callee.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x00408708, FUN_00408708, "regs(esi:ptr[0x267]:bytes) -> (cf, pf, af, zf, sf, of)")

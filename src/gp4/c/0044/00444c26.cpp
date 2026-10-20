#include <gp4/gp4.h>

// Opcode 1 stream handler: consumes a 10-byte record with 0x00445130; unless that found no car (CF set), clears
// bit 2 of the car's byte +0xe5 and runs the one-shot car reset 0x0040f6d0 under pushal / popal. Registers are those
// 0x00445130 leaves; the flags are those of the last callee that ran.
// FUNCTION: GP4 0x00444c26
void __cdecl FUN_00444c26(gp4::Regs* r) {
    gp4::Regs first = *r;
    first.st_in = 0;
    first.st_out = 0;
    gp4::call_regs(0x00445130, first);

    r->eax = first.eax;
    r->ebx = first.ebx;
    r->ecx = first.ecx;
    r->edx = first.edx;
    r->esi = first.esi;
    r->edi = first.edi;
    r->ebp = first.ebp;
    uint32_t flags = first.eflags & 0x8d5u;

    if ((first.eflags & 1u) == 0u) {
        uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(first.esi));
        car[0xe5] = static_cast<uint8_t>(car[0xe5] & 0xfbu);

        gp4::Regs second = first;
        gp4::call_regs(0x0040f6d0, second);
        flags = second.eflags & 0x8d5u;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00444c26, FUN_00444c26, "regs(eax:u32) -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0041104f+flag_007a97a0+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab590+carst_0040f6d0+carno_0066ff6c+stream_0062d1a8")

#include <gp4/gp4.h>

// Looks up the byte at +0x7c of the esi record in the order list at 0x675958 (index search
// 0x516d17). ebx is pushed and restored; eax and the flags are the search result (index and
// CF = 0 when found, eax = 0 and CF = 1 when not).
// FUNCTION: GP4 0x00516d07
void __cdecl FUN_00516d07(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs search = *r;
    search.ebx = 0x00675958u;
    search.eax = (r->eax & 0xffffff00u) | GP4_FIELD(uint8_t, record, 0x7c);
    gp4::call_regs(0x00516d17, search);
    r->eax = search.eax;
    r->eflags = search.eflags;
}
GP4_IMPL(0x00516d07, FUN_00516d07, "regs(esi:ptr[0x84]:u8{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,0x41,0x96,0xc5,0x7f}) -> (eax, cf, pf, af, zf, sf, of); globals=layout=order_00675958")

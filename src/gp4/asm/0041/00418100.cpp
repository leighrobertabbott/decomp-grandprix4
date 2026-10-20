#include <gp4/gp4.h>

// FUNCTION: GP4 0x00418100
// Records the 64-bit value read by 0x433d52 (edx:eax) and its pointer (ecx) in the 0x1c-byte slot
// of the car number held in byte +0x7c (low 6 bits, minus one, clamped to 0x15), then calls
// 0x434044 with eax = 0. Leaves ebx = slot index, eax = 0, flags of 0x434044.
void __cdecl FUN_00418100(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, car, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    uint8_t* slot = GP4_ARRAY(uint8_t, 0x0062ffac) + index * 0x1c;

    gp4::Regs first = *r;
    first.st_in = 0;
    first.st_out = 0;
    gp4::call_regs(0x00433d52, first);
    GP4_FIELD(uint32_t, slot, 0x0c) = first.eax;
    GP4_FIELD(uint32_t, slot, 0x10) = first.edx;
    GP4_FIELD(uint32_t, slot, 0x14) = first.ecx;

    gp4::Regs second = first;
    second.eax = 0;
    gp4::call_regs(0x00434044, second);

    r->eax = 0;
    r->ebx = static_cast<uint32_t>(index);
    r->ecx = first.ecx;
    r->edx = first.edx;
    r->eflags = second.eflags;
}
GP4_IMPL(0x00418100, FUN_00418100, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+carno_0066ff6c")

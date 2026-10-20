#include <gp4/gp4.h>

struct Unknown_00431fa3_Car {
    uint8_t Unknown00[0x7e];
    uint8_t Kind7E;
    uint8_t Unknown7F[0x8d - 0x7f];
    uint8_t Flags8D;
};

// FUNCTION: GP4 0x00431fa3
// Marks the pending group reset as empty ([0x62a96c] = -1) and, when bit 1 of byte +0x8d of the
// record at esi is set, queues one: [0x62a968] = the record base computed by 0x0043207c /
// 0x00432c03 (the car's slot in the segment array), [0x62a96c] = byte +0x7e (the group index),
// then runs the copy 0x00431efb with eax = that index and edx = the base. Without the bit the
// flags are those of the `test` (zero result); otherwise the registers and flags are those
// 0x00431efb leaves (eax, ecx, edx).
void __cdecl FUN_00431fa3(gp4::Regs* r) {
    const Unknown_00431fa3_Car* car = reinterpret_cast<const Unknown_00431fa3_Car*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint32_t, 0x0062a96c) = 0xffffffffu;
    if ((car->Flags8D & 2u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0043207c, call);
    gp4::call_regs(0x00432c03, call);

    call.edx = call.eax;
    call.eax = car->Kind7E;
    GP4_GLOBAL(uint32_t, 0x0062a968) = call.edx;
    GP4_GLOBAL(uint32_t, 0x0062a96c) = call.eax;
    gp4::call_regs(0x00431efb, call);

    r->eax = call.eax;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00431fa3, FUN_00431fa3, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+state_00431efb+state_00432f18+side_00431fa3")

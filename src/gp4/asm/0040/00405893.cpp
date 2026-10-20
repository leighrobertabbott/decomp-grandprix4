#include <gp4/gp4.h>

// FUNCTION: GP4 0x00405893
// Stores the record pointer in edi at 0x00602d84 and 0x00602d88 and the signed word of the car in
// esi at 0x006030d0 (-word +0x24a, or word +0xee when +0x24a is zero), runs the two pushal-wrapped
// routines 0x004057cf and 0x004279ab, then clears 0x006030d0. eax keeps the stored value; every other
// register is restored by their popal and the flags are those 0x004279ab leaves.
void __cdecl FUN_00405893(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint32_t, 0x00602d84) = r->edi;
    GP4_GLOBAL(uint32_t, 0x00602d88) = r->edi;

    int32_t value = GP4_FIELD(int16_t, car, 0x24a);
    if (value != 0) {
        value = -value;
    } else {
        value = GP4_FIELD(int16_t, car, 0xee);
    }
    GP4_GLOBAL(int32_t, 0x006030d0) = value;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = static_cast<uint32_t>(value);
    gp4::call_regs(0x004057cf, call);
    gp4::call_regs(0x004279ab, call);

    GP4_GLOBAL(uint32_t, 0x006030d0) = 0;

    r->eax = call.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00405893, FUN_00405893, "regs(esi:ptr[0x28c]:i16[-1..1], edi:rec[0x8943a0,0x160,0..25]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

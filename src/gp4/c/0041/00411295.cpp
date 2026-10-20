#include <gp4/gp4.h>

// FUNCTION: GP4 0x00411295
// pushal wrapper around the car record pointed to by [0x00679d30]. Unless bit 2 of its byte +0xfe
// is set, 0x0043c784 (esi = the car) decides whether the car is close to its teammate (CF = 1);
// then, unless [0x007c2e75] is set, it queues the 0x00418fd0 event (ecx = 9, eax = edx = 0x1f4)
// and in either case sets bit 0 of byte +0xcd. Finally 0x0041104f runs. Every register comes back
// (popal); the flags are those left by 0x0041104f (the last flag writer).
void __cdecl FUN_00411295(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));

    if ((GP4_FIELD(uint8_t, car, 0xfe) & 4u) == 0u) {
        gp4::call_regs(0x0043c784, state);
        if ((state.eflags & 1u) != 0u) {
            if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u) {
                gp4::Regs event = state;
                event.ecx = 9u;
                event.eax = 0x1f4u;
                event.edx = 0x1f4u;
                gp4::call_regs(0x00418fd0, event);
            }
            GP4_FIELD(uint8_t, car, 0xcd) |= 1u;
        }
    }

    gp4::call_regs(0x0041104f, state);
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x00411295, FUN_00411295, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00418f8f+state_00411109+cars_0066fef0+state_00411017+lap_0043b482+state_0041104f+state_00432330+state_0043c677+segw_0043c608+state_0066d8c0+clock_0066d814+cars_00411295")

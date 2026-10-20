#include <gp4/gp4.h>

// FUNCTION: GP4 0x004157aa
// Per-car update before the steering word is chosen: [0x00638dbc] = 0x4000 (grip cap) and
// [0x0062fbec] = 0, then, unless [0x00622424] is set or bit 5 of the car's byte +0x7a is set, the
// rating 0x004155c7 runs and, when +0xe5 bit 2 is clear and +0x78 bit 1 is set, the gear/flag
// decision 0x0041aea9. The word +0xee is always chosen by 0x004160f2. Every register and the flags
// are what the last callee leaves (esi = car, edi = its track segment).
void __cdecl FUN_004157aa(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00638dbc) = 0x4000u;
    GP4_GLOBAL(uint32_t, 0x0062fbec) = 0u;

    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u && (GP4_FIELD(uint8_t, car, 0x7a) & 0x20u) == 0u) {
        gp4::call_regs(0x004155c7, c);
        if ((GP4_FIELD(uint8_t, car, 0xe5) & 4u) == 0u && (GP4_FIELD(uint8_t, car, 0x78) & 2u) != 0u) {
            gp4::call_regs(0x0041aea9, c);
        }
    }
    gp4::call_regs(0x004160f2, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004157aa, FUN_004157aa, "regs(esi:ptr[0x420]:layout=car_004129fa+rec_00412b88+rec_00426947+car_0041aea9, edi:rec[0x8943a0,0x160,0..26], eax:u32, ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041b950+cars_0066fef0+rel_0042c0c4+state_0043c677+pit_0040d9e6+trk_007ad894+state_00427880+state_00426947+frac_00624790+state_0041242f+thr_007c28bc")

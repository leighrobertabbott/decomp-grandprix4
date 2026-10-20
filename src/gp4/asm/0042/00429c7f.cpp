#include <gp4/gp4.h>

// Advances a pointer into the 0x160-byte track-segment ring at 0x8943a0 by one record, wrapping at the
// lap end [0x866ecc] / pit start [0x891500] / pit end [0x866ef8] exactly as the original does.
static inline uint32_t advanceRing(uint32_t p) {
    p += 0x160u;
    if (p >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
        if (p < GP4_GLOBAL(uint32_t, 0x00891500)) {
            p -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        } else if (p >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            p = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }
    return p;
}

// Car record fields used here.
struct Unknown_00429c7f {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;        // current track segment (ring record)
    uint8_t Unknown20[0x66 - 0x20];
    int16_t Blend66;           // Q14 blend factor
    uint8_t Unknown68[0x9a - 0x68];
    int16_t Word9A;
    uint8_t Unknown9C[0x146 - 0x9c];
    int16_t Word146;
    uint8_t Unknown148[0x1b8 - 0x148];
    uint32_t Segment1B8;       // second track-segment pointer
};

// FUNCTION: GP4 0x00429c7f
// Looks ahead along the segment ring: the blend factor +0x66 is advanced by
// (+0x9a * [0x603854] >> 2) / max(+0x146, 16) and every 0x4000 carried over moves the segment
// pointers +0x1b8 and +0x1c on by one record (at most four). With the clamped factor and the moved
// pointers stored it runs 0x00427760, then puts +0x66, +0x1b8 and +0x1c back. Leaves ebx = the moved
// +0x1b8, ebp = 4 - steps, eax = (eax high : old +0x66), and ecx/edx/flags from the callee.
void __cdecl FUN_00429c7f(gp4::Regs* r) {
    Unknown_00429c7f* rec = reinterpret_cast<Unknown_00429c7f*>(static_cast<uintptr_t>(r->esi));

    const uint16_t oldBlend = static_cast<uint16_t>(rec->Blend66);
    const uint32_t savedEax = (r->eax & 0xffff0000u) | oldBlend;
    const uint32_t oldBack = rec->Segment1B8;
    const uint32_t oldFront = rec->Segment1C;

    uint32_t ebx = oldBack;
    uint32_t edi = oldFront;

    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(rec->Word9A)) * GP4_GLOBAL(int32_t, 0x00603854);
    int32_t divisor = rec->Word146;
    if (divisor < 0x10) {
        divisor = 0x10;
    }
    int32_t remainder;
    int32_t amount = gp4::idiv64(product >> 2, divisor, &remainder);
    amount += static_cast<int32_t>(rec->Blend66);

    uint32_t ebp = 4u;
    while (amount >= 0x4000) {
        ebx = advanceRing(ebx);
        edi = advanceRing(edi);
        amount -= 0x4000;
        if (--ebp == 0u) {
            break;
        }
    }
    if (amount < 0) {
        amount = 0;
    }
    if (amount > 0x4000) {
        amount = 0x4000;
    }

    rec->Blend66 = static_cast<int16_t>(amount);
    rec->Segment1B8 = ebx;
    rec->Segment1C = edi;

    gp4::Regs call = *r;
    call.eax = static_cast<uint32_t>(amount);
    call.ebx = ebx;
    call.edx = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(oldBlend)));
    call.edi = edi;
    call.ebp = ebp;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00427760, call);

    rec->Segment1C = oldFront;
    rec->Segment1B8 = oldBack;
    rec->Blend66 = static_cast<int16_t>(oldBlend);

    r->eax = savedEax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00429c7f, FUN_00429c7f, "regs(eax:u32, esi:ptr[0x41e]:layout=rec_004275d1+car_0043c677+car_00429c7f) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_00427880+ringw_00429c7f")

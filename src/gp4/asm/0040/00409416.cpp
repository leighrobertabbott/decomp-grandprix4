#include <gp4/gp4.h>

struct Unknown_00409416_Source {
    uint8_t Unknown00[0x16];
    int16_t Unknown16;
    uint8_t Unknown18[0x176 - 0x18];
    int16_t Unknown176;
};

struct Unknown_00409416 {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x94 - 0x68];
    int16_t Unknown94;
    uint8_t Unknown96[0x9c - 0x96];
    uint8_t Flags9C;
    uint8_t Unknown9D[0x1b8 - 0x9d];
    Unknown_00409416_Source* Unknown1B8;
};

// Interpolates a 16-bit value between two words of the source block at +0x1b8 by the
// 2.14 fixed-point factor at +0x66 (0 when flag 0x10 at +0x9c is set), stores it at +0x94,
// then lets 0x4030c6 do the same for the second pair.
// FUNCTION: GP4 0x00409416
void __cdecl FUN_00409416(gp4::Regs* r) {
    Unknown_00409416* body = reinterpret_cast<Unknown_00409416*>(r->esi);
    const uint32_t savedEdi = r->edi;

    uint32_t value;
    if ((body->Flags9C & 0x10u) != 0u) {
        value = 0u;
    } else {
        const Unknown_00409416_Source* source = body->Unknown1B8;
        const int32_t low = source->Unknown16;
        const int32_t span = (int32_t)source->Unknown176 - low;
        const int32_t factor = body->Unknown66;
        const int64_t product = (int64_t)span * (int64_t)factor;
        value = (uint32_t)((uint64_t)product >> 14) + (uint32_t)low;
    }
    body->Unknown94 = (int16_t)value;

    gp4::Regs call = *r;
    call.edi = *reinterpret_cast<const uint32_t*>(r->esi + 0x1c);
    call.eax = value;
    gp4::call_regs(0x004030c6, call);

    r->eax = call.eax;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->edi = savedEdi;
    r->eflags = call.eflags;
}
GP4_IMPL(0x00409416, FUN_00409416, "regs(esi:ptr[0x420]:layout=rec_00409416) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_0041d43f_Source {
    uint8_t Unknown00[0xa4];
    int32_t UnknownA4;
    int32_t UnknownA8;
    uint8_t UnknownAC[0x150 - 0xac];
    int32_t Unknown150;
};

// Copies three integers of the esi record (+0xa4, +0x150, +0xa8) as doubles into the vector at
// +0xa0 / +0xa8 / +0xb0 of the body at 0x00622cf8, rotates it with FUN_00401e2e (results at +0xb8 /
// +0xc0 / +0xc8) and rounds the three results into the dwords at 0x0063804c / 0x00638050 / 0x00638054.
// edx is left holding the body address.
// FUNCTION: GP4 0x0041d43f
void __cdecl FUN_0041d43f(gp4::Regs* r) {
    const Unknown_0041d43f_Source* source = reinterpret_cast<const Unknown_0041d43f_Source*>(static_cast<uintptr_t>(r->esi));
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622cf8);

    GP4_FIELD(double, body, 0xa0) = static_cast<double>(source->UnknownA4);
    GP4_FIELD(double, body, 0xa8) = static_cast<double>(source->Unknown150);
    GP4_FIELD(double, body, 0xb0) = static_cast<double>(source->UnknownA8);

    gp4::Regs call = *r;
    call.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401e2e, call);

    GP4_GLOBAL(int32_t, 0x0063804c) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xb8));
    GP4_GLOBAL(int32_t, 0x00638050) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc0));
    GP4_GLOBAL(int32_t, 0x00638054) = gp4::x87::fistp32(GP4_FIELD(double, body, 0xc8));

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
}
GP4_IMPL(0x0041d43f, FUN_0041d43f, "regs(esi:ptr[0x158]:bytes) -> (edx); globals=fuzz:i32")

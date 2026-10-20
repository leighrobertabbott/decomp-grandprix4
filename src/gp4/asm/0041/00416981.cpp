#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

struct Unknown_00416981 {
    uint8_t Unknown00[0x18];
    int32_t Unknown18;
    uint8_t Unknown1C[0x24];
    int32_t Unknown40;
    uint8_t Unknown44[0x234];
    double Angle278;
    double Sin280;
    double Cos288;
    double Unknown290;
};

// FUNCTION: GP4 0x00416981
// angle = atan2([ebx+0x18], [ebx+0x40]); stores angle, sin, cos, then a clamped ratio of the
// vector length to a global via 0x00401ccc.
void __cdecl FUN_00416981(gp4::Regs* r) {
    Unknown_00416981* body = reinterpret_cast<Unknown_00416981*>(static_cast<uintptr_t>(r->ebx));

    const Ext a = gp4::x87::ext_i32(body->Unknown18);
    const Ext b = gp4::x87::ext_i32(body->Unknown40);
    const Ext angle = gp4::x87::fpatan(a, b);
    body->Angle278 = gp4::x87::narrow(angle);
    double sine;
    double cosine;
    gp4::x87::fsincos(gp4::x87::narrow(angle), &sine, &cosine);
    body->Cos288 = cosine;
    body->Sin280 = sine;

    const Ext length = gp4::x87::fsqrt(b * b + a * a);

    gp4::Regs call = *r;
    call.st_in = 3;
    call.st_out = 1;
    gp4::x87::set_in_st(call, 0, gp4::x87::ext(GP4_GLOBAL(double, 0x006f2a8c)));
    gp4::x87::set_in_st(call, 1, gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006034a4)));
    gp4::x87::set_in_st(call, 2, length);
    gp4::call_regs(0x00401ccc, call);
    body->Unknown290 = gp4::x87::narrow(gp4::x87::out_st(call, 0));

    r->eflags = call.eflags;
}
GP4_IMPL(0x00416981, FUN_00416981, "regs(ebx:ptr[0x298]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

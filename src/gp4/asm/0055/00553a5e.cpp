#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

using gp4::x87::Ext;

struct Unknown_00553a5e {
    uint8_t Unknown00[0xec];
    int32_t UnknownEC;
    int32_t UnknownF0;
    uint8_t UnknownF4[4];
    int32_t UnknownF8;
};

// Rotates the vector (eax, 0) by the angle [edi+0xec] * [0x006f2bdc]: eax = round(eax*cos) +
// [edi+0xf0], edx = [edi+0xf8] - round(eax*sin). ecx is saved and restored; the flags are those
// of the final `add edx, eax` (the pops do not touch them). Same maths as 0x0040cee7.
// FUNCTION: GP4 0x00553a5e
void __cdecl FUN_00553a5e(gp4::Regs* r) {
    const Unknown_00553a5e* body = reinterpret_cast<const Unknown_00553a5e*>(static_cast<uintptr_t>(r->edi));
    const int32_t length = static_cast<int32_t>(r->eax);

    const Ext angle = gp4::x87::ext_i32(body->UnknownEC) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    Ext sine;
    Ext cosine;
    gp4::x87::fsincos(angle, &sine, &cosine);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = length;
    const int32_t across = gp4::x87::fistp32(cosine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = across;
    const uint32_t resultEax = static_cast<uint32_t>(across) + static_cast<uint32_t>(body->UnknownF0);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = length;
    const int32_t down = gp4::x87::fistp32(gp4::x87::fchs(sine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4))));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = down;
    const uint32_t base = static_cast<uint32_t>(body->UnknownF8);
    const uint32_t resultEdx = base + static_cast<uint32_t>(down);

    r->eax = resultEax;
    r->edx = resultEdx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(base, static_cast<uint32_t>(down)));
}
GP4_IMPL(0x00553a5e, FUN_00553a5e, "regs(eax:u32, edi:ptr[0x100]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Two-step transform of the track positions (fixed 16.16 words): the dwords 0x00870638 / 0x0087063c receive the
// words of the record [0x00874b1c] shifted into the high half, 0x00553d3d copies that record's vector (+0xf0..)
// to 0x00870640, and - when the byte [0x0086857a] is set - the offsets [0x00874b24] / [0x00874b28] (x 0x100) are
// added to its first and third component. The vector is copied to 0x0087064c.., then the words of the record
// [0x00874b20] are subtracted from 0x00870638 / 0x0087063c, 0x00553d3d loads that record's vector and the
// first copy is subtracted from it componentwise (0x0087064c.. = new - old). eax/ebx/ecx/edi are left as the
// loop leaves them (ebx = 0x0087064c, ecx = 0, edi = [0x00874b20]); the flags are those of the last
// `add ebx, 4`, a constant.
// FUNCTION: GP4 0x00559411
void __cdecl FUN_00559411(gp4::Regs* r) {
    uint32_t* vec = &GP4_GLOBAL(uint32_t, 0x00870640);

    const uint16_t* src = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00874b1c)));
    GP4_GLOBAL(uint32_t, 0x00870638) = static_cast<uint32_t>(src[0]) << 16;
    GP4_GLOBAL(uint32_t, 0x0087063c) = static_cast<uint32_t>(src[1]) << 16;

    r->edi = GP4_GLOBAL(uint32_t, 0x00874b1c);
    gp4::call_regs(0x00553d3d, *r);

    if ((GP4_GLOBAL(uint8_t, 0x0086857a) & 0xffu) != 0u) {
        vec[0] += GP4_GLOBAL(uint32_t, 0x00874b24) << 8;
        vec[2] += GP4_GLOBAL(uint32_t, 0x00874b28) << 8;
    }
    vec[3] = vec[0];
    vec[4] = vec[1];
    vec[5] = vec[2];

    const uint16_t* dst = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00874b20)));
    r->edi = GP4_GLOBAL(uint32_t, 0x00874b20);
    GP4_GLOBAL(uint32_t, 0x00870638) -= static_cast<uint32_t>(dst[0]) << 16;
    GP4_GLOBAL(uint32_t, 0x0087063c) -= static_cast<uint32_t>(dst[1]) << 16;
    gp4::call_regs(0x00553d3d, *r);

    uint32_t eax = r->eax;
    for (uint32_t i = 0; i < 3; i++) {
        eax = vec[i + 3] - vec[i];
        vec[i + 3] = eax;
    }
    r->eax = eax;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(vec)) + 12u;
    r->ecx = 0;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(vec)) + 8u, 4u));
}
GP4_IMPL(0x00559411, FUN_00559411, "regs() -> (eax, ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_00554817")

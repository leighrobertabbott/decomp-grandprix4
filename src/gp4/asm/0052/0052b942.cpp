#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: selects entry eax of the 0x34-byte table at 0x7ab5b4, copies its first 0x20 bytes
// to 0x7ab548 (`rep movsb`, direction flag clear) and its bytes +0x32/+0x33 to 0x7ab568/0x7ab569.
// popal restores every register; the flags stay those of `add eax, 0x7ab5b4` (eax = index * 0x34).
// FUNCTION: GP4 0x0052b942
void __cdecl FUN_0052b942(gp4::Regs* r) {
    const uint32_t offset = r->eax * 0x34u;
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(offset + 0x007ab5b4u);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007ab548);

    for (uint32_t i = 0; i != 0x20u; ++i) {
        dst[i] = entry[i];
    }
    GP4_GLOBAL(uint8_t, 0x007ab568) = entry[0x32];
    GP4_GLOBAL(uint8_t, 0x007ab569) = entry[0x33];

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x007ab5b4u));
}
GP4_IMPL(0x0052b942, FUN_0052b942, "regs(eax:u32[0..7]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=presets_007ab5b4")

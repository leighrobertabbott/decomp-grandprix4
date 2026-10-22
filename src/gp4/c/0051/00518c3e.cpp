#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sound back-end hook with a re-entry guard (byte [0x0088fbb4], 0xff while it runs). Unless the guard is
// set, it clamps edx (signed) to 0..0x80, replaces it with the dword [0x007a8eec + 4 * edx] and passes
// ([0x0088f948 + 4 * eax], that value) to the empty stdcall stub 0x00509ec0. eax is pushed and popped, so it
// comes back unchanged; edx ends up as the table value. The flags are those of the
// `cmp byte [0x0088fbb4], 0xff` on the guarded path, else of the `cmp edx, 0x80` (the stub is a bare
// `ret 8`).
// FUNCTION: GP4 0x00518c3e
void __cdecl FUN_00518c3e(gp4::Regs* r) {
    const uint32_t guard = GP4_GLOBAL(uint8_t, 0x0088fbb4);
    if (guard == 0xffu) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(guard, 0xffu));
        return;
    }
    GP4_GLOBAL(uint8_t, 0x0088fbb4) = 0xffu;

    int32_t index = static_cast<int32_t>(r->edx);
    if (index < 0) {
        index = 0;
    }
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(index), 0x80u);
    if (index > 0x80) {
        index = 0x80;
    }
    const uint32_t value = GP4_ARRAY(uint32_t, 0x007a8eec)[index];
    const uint32_t sample = GP4_ARRAY(uint32_t, 0x0088f948)[r->eax];
    gp4::stdcall_<0x00509ec0, void>(sample, value);
    GP4_GLOBAL(uint8_t, 0x0088fbb4) = 0u;

    r->edx = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00518c3e, FUN_00518c3e, "regs(eax:u32[0..15], edx:i32[-5..0x90]) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=chan_005188f5")

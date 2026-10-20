#include <gp4/gp4.h>
#include <gp4/flags.h>

// Opcode-stream writer: emits opcode byte 7 followed by the mask byte at 0x007c2ead at the write cursor
// (pointer at 0x0062d1a0) and advances the cursor by 2. pushal/popal keep every GPR; the flags are those of
// the final "add dword [cursor], 2".
// FUNCTION: GP4 0x00444e03
void __cdecl FUN_00444e03(gp4::Regs* r) {
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    uint8_t* cursor = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(before));
    cursor[0] = 7;
    cursor[1] = GP4_GLOBAL(uint8_t, 0x007c2ead);
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = before + 2u;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
}
GP4_IMPL(0x00444e03, FUN_00444e03, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stream_0062d1a0")

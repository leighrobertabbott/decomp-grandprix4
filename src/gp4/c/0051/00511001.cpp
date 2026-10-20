#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the dword 0x007ad894, the byte 0x007c2e81 and the 0x176 bytes from 0x007c2f2d (a `loop` of
// `mov byte [esi], 0 ; inc esi`). esi ends one past the block, ecx = 0 and the flags are those of the
// last `inc esi` (CF kept from the caller).
// FUNCTION: GP4 0x00511001
void __cdecl FUN_00511001(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x007ad894) = 0u;
    GP4_GLOBAL(uint8_t, 0x007c2e81) = 0u;
    uint8_t* block = GP4_ARRAY(uint8_t, 0x007c2f2d);
    for (uint32_t i = 0; i != 0x176u; ++i) {
        block[i] = 0u;
    }
    const uint32_t last = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(block)) + 0x175u;
    r->esi = last + 1u;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(last, r->eflags));
}
GP4_IMPL(0x00511001, FUN_00511001, "regs(cf:u32[0..1]) -> (ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pre_00511001")

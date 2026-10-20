#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 1 the cursor esi just skips 12
// bytes. In mode 1 the six words at esi are copied into entry [0x007ad894] (circuit number) of the
// 12-byte-per-circuit table at 0x006f2e94 (`rep movsw`, esi advances by 12). Leaves eax = the table
// offset (circuit * 12), ecx = 0 and the flags of `add edi, eax` (edi is restored).
// FUNCTION: GP4 0x005573d3
void __cdecl FUN_005573d3(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = esi + 12u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 12u));
        return;
    }
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x007ad894) * 12u;
    uint16_t* const dst = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(0x006f2e94u + offset));
    for (uint32_t i = 0; i < 6u; ++i) {
        dst[i] = GP4_FIELD(uint16_t, esi, i * 2u);
    }
    r->eax = offset;
    r->ecx = 0u;
    r->esi = esi + 12u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x006f2e94u, offset));
}
GP4_IMPL(0x005573d3, FUN_005573d3, "regs(eax:u32, ecx:u32, esi:ptr[0x14]:bytes) -> (eax, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+trk_007ad894")

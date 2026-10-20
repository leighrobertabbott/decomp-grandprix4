#include <gp4/gp4.h>
#include <gp4/flags.h>

// al += (byte [edx+9] >> 2) when ecx < 4 (signed), al -= it otherwise. Only the low byte of eax
// changes; the flags are those of the 8-bit add/sub. ebx/ecx/edx are saved and restored.
// FUNCTION: GP4 0x0055b74b
void __cdecl FUN_0055b74b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edx));
    const uint32_t delta = static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 9) >> 2);
    const uint32_t al = r->eax & 0xffu;
    uint32_t result;
    uint32_t flags;
    if (static_cast<int32_t>(r->ecx) < 4) {
        result = (al + delta) & 0xffu;
        flags = gp4::flags::add8(al, delta);
    } else {
        result = (al - delta) & 0xffu;
        flags = gp4::flags::sub8(al, delta);
    }
    r->eax = (r->eax & 0xffffff00u) | result;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055b74b, FUN_0055b74b, "regs(eax:u32, ecx:i32[0..7], edx:ptr[0x40]:bytes) -> (eax, cf, pf, af, zf, sf, of)")

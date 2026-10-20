#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up record eax of the 0x18-byte frame table at 0x008174f8. When the table is not loaded (byte
// 0x0081cd90 is zero) it returns with CF set and the other flags of `cmp byte [0x81cd90], 0`; otherwise
// ebx, eax, ebp and ecx are loaded from the record's first four dwords and the flags are those of the
// `add eax, 0x8174f8` that formed its address. esi is pushed and popped.
// FUNCTION: GP4 0x00527eee
void __cdecl FUN_00527eee(gp4::Regs* r) {
    const uint32_t loaded = GP4_GLOBAL(uint8_t, 0x0081cd90);
    if (loaded == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(loaded, 0u) | gp4::flags::CF);
        return;
    }
    const uint32_t offset = r->eax * 0x18u;
    const uint32_t record = offset + 0x008174f8u;
    r->ebx = GP4_FIELD(uint32_t, record, 0);
    r->eax = GP4_FIELD(uint32_t, record, 4);
    r->ebp = GP4_FIELD(uint32_t, record, 8);
    r->ecx = GP4_FIELD(uint32_t, record, 0xc);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x008174f8u));
}
GP4_IMPL(0x00527eee, FUN_00527eee, "regs(eax:u32[0..16]) -> (eax, ebx, ebp, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=frames_008174f8")

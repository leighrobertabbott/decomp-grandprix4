#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Assertion check on the mask dword at +0x2c of a ring record (edi): bit 0x200 must not come with
// bit 0x1000000, nor bit 0x100 with bit 0x800000 (each violation is an `int3`). Inside
// pushal/popal, so only the flags survive: those of the last `test eax, mask` executed.
// FUNCTION: GP4 0x0055659b
void __cdecl FUN_0055659b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t mask = GP4_FIELD(uint32_t, record, 0x2c);
    uint32_t arith = gp4::flags::logic32(mask & 0x200u);
    if ((mask & 0x200u) != 0u) {
        arith = gp4::flags::logic32(mask & 0x1000000u);
        if ((mask & 0x1000000u) != 0u) {
            __debugbreak();
        }
    }
    arith = gp4::flags::logic32(mask & 0x100u);
    if ((mask & 0x100u) != 0u) {
        arith = gp4::flags::logic32(mask & 0x800000u);
        if ((mask & 0x800000u) != 0u) {
            __debugbreak();
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055659b, FUN_0055659b, "regs(edi:ptr[0x40]:u32{=0,0x200,0x100,0x300,0x1000,0x40000,0x2000000,0x400000}) -> (cf, pf, af, zf, sf, of)")

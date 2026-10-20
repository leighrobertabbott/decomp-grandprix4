#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Address of the ring record selected by the low 14 bits of ax (record size 0x160, array at 0x008943a0),
// clamped to the last of the [0x00868558] records (signed compare). Bit 15 without bit 14 is an assertion
// (int3). Exit: edx:eax = index * 0x160 (full 64-bit product, edx = high dword), edi = 0x008943a0 + eax,
// flags of that `add edi, eax`.
// FUNCTION: GP4 0x0055ac8a
void __cdecl FUN_0055ac8a(gp4::Regs* r) {
    const uint32_t ax = r->eax & 0xffffu;
    if ((ax & 0x4000u) == 0u && (ax & 0x8000u) != 0u) {
        __debugbreak();
    }

    uint32_t index = r->eax & 0x3fffu;
    const uint32_t last = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868558)) - 1u;
    if (static_cast<int32_t>(index) > static_cast<int32_t>(last)) {
        index = last;
    }

    const uint64_t product = static_cast<uint64_t>(index) * 0x160u;
    const uint32_t low = static_cast<uint32_t>(product);
    r->eax = low;
    r->edx = static_cast<uint32_t>(product >> 32);
    r->edi = 0x008943a0u + low;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0x008943a0u, low));
}
GP4_IMPL(0x0055ac8a, FUN_0055ac8a, "regs(eax:u32[0..0x7fff]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

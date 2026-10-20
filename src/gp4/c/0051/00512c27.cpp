#include <gp4/gp4.h>
#include <gp4/flags.h>

// The compiler would fold `address - address` into a constant and hide both globals from the shape audit.
static __declspec(noinline) uint32_t addrSpan(const uint8_t* end, const uint8_t* start) {
    return static_cast<uint32_t>(end - start);
}

// Stores the size of the saved-state block 0x007ad890..0x007c3ae4 (0x16254) in [0x007a93d4] and a derived offset in
// [0x007a93d8]: region size from 0x0040809b + that size + (0x006975bc - 0x0066d7a4) + 3 * [0x006975c4] + 0x3840 +
// 0x002835c0 + 6 + 0x0001c200. ebx is saved and restored, edx ends as the block size (the popped argument).
// The flags are those of the final `add eax, 0x1c200`.
// FUNCTION: GP4 0x00512c27
void __cdecl FUN_00512c27(gp4::Regs* r) {
    const uint32_t blockSize = addrSpan(GP4_ARRAY(uint8_t, 0x007c3ae4), GP4_ARRAY(uint8_t, 0x007ad890));
    GP4_GLOBAL(uint32_t, 0x007a93d4) = blockSize;

    uint32_t eax = gp4::cdecl_<0x0040809b, uint32_t>();
    eax += blockSize;
    eax += addrSpan(GP4_ARRAY(uint8_t, 0x006975bc), GP4_ARRAY(uint8_t, 0x0066d7a4));
    eax += GP4_GLOBAL(uint32_t, 0x006975c4);
    eax += GP4_GLOBAL(uint32_t, 0x006975c4);
    eax += GP4_GLOBAL(uint32_t, 0x006975c4);
    eax += 0x708u << 3;
    eax += 0x002835c0u;
    eax += 2u;
    eax += 4u;
    const uint32_t arith = gp4::flags::add32(eax, 0x0001c200u);
    eax += 0x0001c200u;
    GP4_GLOBAL(uint32_t, 0x007a93d8) = eax;

    r->eax = eax;
    r->edx = blockSize;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00512c27, FUN_00512c27, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

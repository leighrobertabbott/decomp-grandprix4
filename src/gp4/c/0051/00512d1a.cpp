#include <gp4/gp4.h>
#include <gp4/flags.h>

// The compiler would fold `address - address` into a constant and hide both globals from the shape audit.
static __declspec(noinline) uint32_t addrOf(const uint8_t* p) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// Reads the dword at row [0x0088f624] of the slot area: index * pitch [0x006975c4] + base [0x007a8e18]
// + (0x0066d830 - 0x0066d810) goes to [0x0088f604] and eax. edx is saved and restored (the 64-bit
// product's high half); the flags are those of the final `sub eax, 0x0066d810`.
// FUNCTION: GP4 0x00512d1a
void __cdecl FUN_00512d1a(gp4::Regs* r) {
    const int32_t index = GP4_GLOBAL(int32_t, 0x0088f624);
    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x006975c4);
    uint32_t address = static_cast<uint32_t>(index) * pitch;
    address += GP4_GLOBAL(uint32_t, 0x007a8e18);
    address += addrOf(GP4_ARRAY(uint8_t, 0x0066d830));
    const uint32_t start = addrOf(GP4_ARRAY(uint8_t, 0x0066d810));
    const uint32_t arith = gp4::flags::sub32(address, start);
    address -= start;
    const uint32_t value = GP4_FIELD(uint32_t, address, 0);
    GP4_GLOBAL(uint32_t, 0x0088f604) = value;
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00512d1a, FUN_00512d1a, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+idx_00512d1a")

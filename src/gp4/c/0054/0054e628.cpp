#include <gp4/gp4.h>
#include <gp4/flags.h>

// The 64-bit difference of the fresh counter qword at 0x00838c5c and the start qword at 0x00838c54
// (`mov eax, [c]; mov edx, [c+4]; sub eax, [s]; sbb edx, [s+4]`).
static __declspec(noinline) uint64_t counterDelta() {
    return GP4_GLOBAL(uint64_t, 0x00838c5c) - GP4_GLOBAL(uint64_t, 0x00838c54);
}

// High-resolution timer fraction with scale 2^10 (no overflow guard, unlike its siblings 0x0054e665 and
// 0x0054e6af): reads the performance counter into the qword 0x00838c5c (thunk 0x005a48be:
// QueryPerformanceCounter), subtracts the start qword 0x00838c54, shifts the 64-bit difference left by 10
// (`shld edx, eax, 10; shl eax, 10`) and divides it by the frequency dword [0x00838c4c]; the quotient is
// returned in eax. ebx, ecx and edx are saved and restored. The flags are those of the `shl eax, 10`
// (DIV leaves them): CF = bit 22 of the low dword, OF = bit31((low << 9) ^ (low << 10)), AF = 0.
// FUNCTION: GP4 0x0054e628
void __cdecl FUN_0054e628(gp4::Regs* r) {
    gp4::stdcall_<0x005a48be, int>(&GP4_GLOBAL(uint32_t, 0x00838c5c));
    const uint64_t delta = counterDelta();
    const uint32_t low = static_cast<uint32_t>(delta);

    const uint32_t shifted = low << 10;
    const uint32_t previous = low << 9;
    uint32_t flags = gp4::flags::szp(shifted, 32);
    if ((previous & 0x80000000u) != 0u) {
        flags |= gp4::flags::CF;
    }
    if (((previous ^ shifted) & 0x80000000u) != 0u) {
        flags |= gp4::flags::OF;
    }

    uint32_t remainder;
    r->eax = gp4::div64(delta << 10, GP4_GLOBAL(uint32_t, 0x00838c4c), &remainder);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054e628, FUN_0054e628, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=timer_00838c4c")

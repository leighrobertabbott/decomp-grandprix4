#include <gp4/gp4.h>
#include <gp4/flags.h>

// Refreshes the cached stream window dwords from three 0x28-byte descriptors (flag byte +0xd, bit 0x80 = valid):
// descriptor 0x829840 fills 0x822258/0x82225c/0x822260/0x822264 with its dword +0x14, +0x14 + 0xea600 (twice) and
// +0x14 + 0x1d4c00, or all four with -1 when not valid; descriptor 0x829890 (when valid) fills 0x82fd4a, 0x82fd38,
// 0x82fd3c with its dword +0x14 and 0x82fd4e with +0x14 + its dword +0x10; descriptor 0x829868 (when valid) runs the
// pool reset 0x0053872f. eax and edx are saved and restored; the flags are those of the last `test` or of the reset.
// FUNCTION: GP4 0x00538d70
void __cdecl FUN_00538d70(gp4::Regs* r) {
    uint8_t* const first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00829840u));
    if ((GP4_FIELD(uint8_t, first, 0xd) & 0x80u) != 0u) {
        uint32_t value = GP4_FIELD(uint32_t, first, 0x14);
        GP4_GLOBAL(uint32_t, 0x00822258) = value;
        value += 0xea600u;
        GP4_GLOBAL(uint32_t, 0x0082225c) = value;
        GP4_GLOBAL(uint32_t, 0x00822260) = value;
        value += 0xea600u;
        GP4_GLOBAL(uint32_t, 0x00822264) = value;
    } else {
        GP4_GLOBAL(uint32_t, 0x00822258) = 0xffffffffu;
        GP4_GLOBAL(uint32_t, 0x0082225c) = 0xffffffffu;
        GP4_GLOBAL(uint32_t, 0x00822260) = 0xffffffffu;
        GP4_GLOBAL(uint32_t, 0x00822264) = 0xffffffffu;
    }

    uint8_t* const second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00829890u));
    if ((GP4_FIELD(uint8_t, second, 0xd) & 0x80u) != 0u) {
        uint32_t value = GP4_FIELD(uint32_t, second, 0x14);
        GP4_GLOBAL(uint32_t, 0x0082fd4a) = value;
        GP4_GLOBAL(uint32_t, 0x0082fd38) = value;
        GP4_GLOBAL(uint32_t, 0x0082fd3c) = value;
        value += GP4_FIELD(uint32_t, second, 0x10);
        GP4_GLOBAL(uint32_t, 0x0082fd4e) = value;
    }

    uint8_t* const third = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x00829868u));
    const uint32_t flagsByte = GP4_FIELD(uint8_t, third, 0xd) & 0x80u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagsByte));
    if (flagsByte != 0u) {
        gp4::call_regs(0x0053872f, *r);
    }
}
GP4_IMPL(0x00538d70, FUN_00538d70, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pool_00538a24+desc_00538d70")

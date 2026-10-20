#include <gp4/gp4.h>
#include <gp4/flags.h>

struct Unknown_00829cac {
    int32_t Index00;
    uint8_t Unknown04[4];
    int32_t Zero08;
    int32_t Zero0C;
    uint8_t Unknown10[8];
};

// Resets the table of 0x400 24-byte entries at 0x829cac (index, zero, zero) and copies the dword at
// 0x829868+0x14 to 0x82fcac. eax/edx are saved and restored by the original; the flags are those of
// the final `cmp eax, 0x400`.
// FUNCTION: GP4 0x0053873a
void __cdecl FUN_0053873a(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0082fcac) = GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x00829868), 0x14);
    Unknown_00829cac* entry = GP4_ARRAY(Unknown_00829cac, 0x00829cac);
    int32_t index = 0;
    do {
        entry->Index00 = index;
        entry->Zero0C = 0;
        entry->Zero08 = 0;
        entry = entry + 1;
        index = index + 1;
    } while (index != 0x400);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(static_cast<uint32_t>(index), 0x400u));
}
GP4_IMPL(0x0053873a, FUN_0053873a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

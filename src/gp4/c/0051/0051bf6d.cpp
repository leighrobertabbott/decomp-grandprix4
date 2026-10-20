#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry address of the 12-byte circuit record 0x007c3ae4 + 12 * circuit; kept out of line so the table base stays
// visible as the operand of the original `add eax, 0x7c3ae4`.
static __declspec(noinline) uint32_t recordAddr(uint32_t scaledIndex, uint32_t base) {
    return scaledIndex + base;
}

// Copies the three bytes +9, +10 and +11 of the current circuit's 12-byte record (table 0x007c3ae4, circuit number
// [0x007ad894]) into the dwords 0x00a69878 / 0x00a6987c / 0x00a69880. Exit: eax = the record address, ebx = the
// byte +11; the flags are those of the `add eax, 0x7c3ae4`.
// FUNCTION: GP4 0x0051bf6d
void __cdecl FUN_0051bf6d(gp4::Regs* r) {
    const uint32_t scaled = GP4_GLOBAL(uint32_t, 0x007ad894) * 0xcu;
    const uint32_t record = recordAddr(scaled, 0x007c3ae4u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(scaled, 0x007c3ae4u));

    uint32_t value = GP4_FIELD(uint8_t, record, 9);
    GP4_GLOBAL(uint32_t, 0x00a69878) = value;
    value = GP4_FIELD(uint8_t, record, 0xa);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
    value = GP4_FIELD(uint8_t, record, 0xb);
    GP4_GLOBAL(uint32_t, 0x00a69880) = value;

    r->eax = record;
    r->ebx = value;
}
GP4_IMPL(0x0051bf6d, FUN_0051bf6d, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894")

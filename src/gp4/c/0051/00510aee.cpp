#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores a car-slot value in the dword 0x00a69878 from the selected car record [0x00679d30]. With the byte
// flag 0x007c2e75 clear the value is byte +0x73 minus one; otherwise it is the car number (byte +0x7c & 0x3f)
// minus one, and its low byte is replaced by the byte of the table at 0x007c2b24 at that index (the
// upper 24 bits keep the index, which is 0xffffff when the number is zero). pushal/popal keep every
// register; the flags are those of the `dec eax` (CF is 0 from the preceding `cmp byte, 0` / `and al, 0x3f`).
// FUNCTION: GP4 0x00510aee
void __cdecl FUN_00510aee(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    uint32_t before;
    uint32_t value;
    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u) {
        before = GP4_FIELD(uint8_t, car, 0x7c) & 0x3fu;
        value = before - 1u;
        value = (value & 0xffffff00u) | GP4_ARRAY(const uint8_t, 0x007c2b24)[static_cast<int32_t>(value)];
    } else {
        before = GP4_FIELD(uint8_t, car, 0x73);
        value = before - 1u;
    }
    GP4_GLOBAL(uint32_t, 0x00a69878) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(before, 0u));
}
GP4_IMPL(0x00510aee, FUN_00510aee, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper: restores saved slot (argument + 1, so 1..3) through 0x0051006c, then stores
// the selected car's number minus one, clamped to 0x15, in the dword 0x00a69878:
// min((byte [0x00679d30] + 0x7c & 0x3f) - 1, 0x15) as a signed compare. Every register comes back
// unchanged and the callers ignore the flags, so this is a plain stdcall with a void result.
// FUNCTION: GP4 0x00510abf
void __stdcall FUN_00510abf(int slot) {
    gp4::Regs c = {};
    c.eax = static_cast<uint32_t>(slot) + 1u;
    gp4::call_regs(0x0051006c, c);
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, car, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    GP4_GLOBAL(int32_t, 0x00a69878) = index;
}
GP4_IMPL(0x00510abf, FUN_00510abf, "stdcall(i32[0..2]) -> void; globals=layout=save_00510309+carbg_0066fef0+slotcar_00510abf")

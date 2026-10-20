#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t SlotByte(int32_t index) {
    return GP4_ARRAY(uint8_t, 0x0066ec4a)[index];
}

// pushal/popal routine: index = (byte +0x7c of the selected car record [0x00679d30] & 0x3f) - 1,
// clamped above to 0x15 (signed compare; -1 is kept); [0x00a6987c] = the byte 0x0066ec4a[index].
// Every GPR is restored; the flags are those of the final `and eax, 0xff`.
// FUNCTION: GP4 0x00516725
void __cdecl FUN_00516725(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00679d30);
    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, record, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    const uint32_t value = SlotByte(index);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00516725, FUN_00516725, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ptr_00679d30")

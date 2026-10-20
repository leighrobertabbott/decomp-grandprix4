#include <gp4/gp4.h>

static __declspec(noinline) uint16_t TableWord7a8dc0(int32_t index) {
    return GP4_ARRAY(uint16_t, 0x007a8dc0)[index];
}

// Copies word [index] of the table at 0x007a8dc0, with index = the signed word [0x007ad89e], to the
// word 0x00679b5a. eax is clobbered (callers ignore it).
// FUNCTION: GP4 0x0052241f
void __cdecl FUN_0052241f() {
    const int32_t index = GP4_GLOBAL(int16_t, 0x007ad89e);
    GP4_GLOBAL(uint16_t, 0x00679b5a) = TableWord7a8dc0(index);
}
GP4_IMPL(0x0052241f, FUN_0052241f, "cdecl() -> void; globals=layout=diff_0052241f")

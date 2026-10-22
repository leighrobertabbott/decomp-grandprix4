#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004597f0 { uint32_t Words[10]; };
#pragma pack(pop)

// Clears the first ten dwords (0x28 bytes) of the object and returns it.
// FUNCTION: GP4 0x004597f0
Unknown_004597f0* __fastcall FUN_004597f0(Unknown_004597f0* self, void* /*edx*/) {
    for (int i = 0; i < 8; i++) {
        self->Words[i] = 0;
    }
    self->Words[8] = 0;
    self->Words[9] = 0;
    return self;
}
GP4_IMPL(0x004597f0, FUN_004597f0, "thiscall(ptr[0x100]:bytes) -> i32")

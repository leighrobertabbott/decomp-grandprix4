#include <gp4/gp4.h>

struct Unknown_005c0250 { uint8_t Unknown00[0x100]; };

// Returns its `this` pointer: `mov eax, ecx; ret`.
// FUNCTION: GP4 0x005c0250
Unknown_005c0250* __fastcall FUN_005c0250(Unknown_005c0250* self, void* /*edx*/) { return self; }
GP4_IMPL(0x005c0250, FUN_005c0250, "thiscall(ptr[0x100]:bytes) -> i32")

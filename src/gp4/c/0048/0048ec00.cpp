#include <gp4/gp4.h>

struct Unknown_0048ec00 { uint8_t Unknown00[0x100]; };

// Returns its `this` pointer: `mov eax, ecx; ret`.
// FUNCTION: GP4 0x0048ec00
Unknown_0048ec00* __fastcall FUN_0048ec00(Unknown_0048ec00* self, void* /*edx*/) { return self; }
GP4_IMPL(0x0048ec00, FUN_0048ec00, "thiscall(ptr[0x100]:bytes) -> i32")

#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d9200 { uint8_t Unknown00[0x12c]; int32_t Value12c; };
#pragma pack(pop)

// Returns the dword at +0x12c of the object; ignores its stack argument (`ret 4`).
// FUNCTION: GP4 0x005d9200
int __fastcall FUN_005d9200(Unknown_005d9200* self, void* /*edx*/, int32_t /*unused*/) {
    return self->Value12c;
}
GP4_IMPL(0x005d9200, FUN_005d9200, "thiscall(ptr[0x140]:bytes, i32) -> i32")

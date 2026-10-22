#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00448e40 {
    uint8_t Unknown00[0x58];
    int32_t Mode58;
    int32_t Pass5c;
};
#pragma pack(pop)

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

// Picks the shader method: defaults to mode 2, overridden by the global at 0x009fa958 unless that is -1.
// Modes 2 (two-pass dual-texture) and 4 (fallback method A) set the pass count at +0x5c to 2 and log a
// debug string. Returns the chosen mode.
// FUNCTION: GP4 0x00448e40
int32_t __fastcall FUN_00448e40(Unknown_00448e40* self, void* /*edx*/, int32_t /*unused*/) {
    self->Mode58 = 2;
    if (GP4_GLOBAL(int32_t, 0x009fa958) != -1) {
        self->Mode58 = GP4_GLOBAL(int32_t, 0x009fa958);
    }
    switch (self->Mode58) {
    case 2:
        self->Pass5c = 2;
        GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0063e248));
        break;
    case 4:
        self->Pass5c = 2;
        GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0063e284));
        return self->Mode58;
    }
    return self->Mode58;
}
GP4_IMPL(0x00448e40, FUN_00448e40, "thiscall(ptr[0x80]:bytes, i32) -> i32; globals=fuzz:i32")

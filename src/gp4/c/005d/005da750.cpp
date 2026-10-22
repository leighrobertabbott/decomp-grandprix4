#include <gp4/gp4.h>

typedef void (__stdcall* BinkGoto_t)(void* bink, int32_t frame, int32_t flags);

#pragma pack(push, 1)
struct Unknown_005da750 { void* Bink00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Seeks the Bink movie handle stored at +0 to the given frame: BinkGoto(bink, frame, 0).
// FUNCTION: GP4 0x005da750
void __fastcall FUN_005da750(Unknown_005da750* self, void* /*edx*/, int32_t frame) {
    GP4_IAT(BinkGoto_t, 0x005f03d0)(self->Bink00, frame, 0);
}
GP4_IMPL(0x005da750, FUN_005da750, "thiscall(ptr[0x100]:bytes, i32) -> void; emu=skip:win32 _BinkGoto@12")

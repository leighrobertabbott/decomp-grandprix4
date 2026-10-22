#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004dc1d0 { int32_t Unknown00; int32_t Unknown04; int32_t Unknown08; };
#pragma pack(pop)

// Zeroes the three dwords of the record.
// FUNCTION: GP4 0x004dc1d0
void __cdecl FUN_004dc1d0(Unknown_004dc1d0* rec) {
    rec->Unknown08 = 0;
    rec->Unknown00 = 0;
    rec->Unknown04 = 0;
}
GP4_IMPL(0x004dc1d0, FUN_004dc1d0, "cdecl(ptr[0x10]:bytes) -> void")

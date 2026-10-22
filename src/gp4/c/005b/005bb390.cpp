#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bb390_Inner { int32_t Value00; };
struct Unknown_005bb390_Mid { uint8_t Unknown00[8]; Unknown_005bb390_Inner* Inner08; };
struct Unknown_005bb390 { uint8_t Unknown00[4]; Unknown_005bb390_Mid* Mid04; };
#pragma pack(pop)

// Follows self->+4 -> +8 and returns the first dword there; 0 when either link is null.
// FUNCTION: GP4 0x005bb390
int32_t __fastcall FUN_005bb390(Unknown_005bb390* self, void* /*edx*/) {
    Unknown_005bb390_Mid* mid = self->Mid04;
    if (mid != nullptr) {
        Unknown_005bb390_Inner* inner = mid->Inner08;
        if (inner != nullptr) {
            return inner->Value00;
        }
    }
    return 0;
}
GP4_IMPL(0x005bb390, FUN_005bb390, "thiscall(ptr[0x10]:{4:nptr[0x10]:{8:nptr[0x10]:bytes}}) -> i32")

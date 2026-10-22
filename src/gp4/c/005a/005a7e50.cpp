#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a7e50 {
    uint8_t  Unknown00[6];
    uint16_t Count06;
    uint8_t  Unknown08[4];
    int16_t* Values0C;
};
#pragma pack(pop)

// Searches the first Count06 words of the table at +0xc for the (low word of the) id; on a hit stores
// its index through the out pointer and returns 0, otherwise returns 0x3ea.
// FUNCTION: GP4 0x005a7e50
int32_t __fastcall FUN_005a7e50(Unknown_005a7e50* self, void* /*edx*/, int16_t id, uint16_t* out) {
    uint16_t i = 0;
    if (self->Count06 != 0) {
        do {
            if (self->Values0C[i] == id) {
                *out = i;
                return 0;
            }
            i = (uint16_t)(i + 1);
        } while (i < self->Count06);
    }
    return 0x3ea;
}
GP4_IMPL(0x005a7e50, FUN_005a7e50, "thiscall(ptr[0x20]:{6:u16[0..8], 0xc:ptr[0x20]:i16[0..3]}, i32[-1..5], ptr[4]:bytes) -> i32")

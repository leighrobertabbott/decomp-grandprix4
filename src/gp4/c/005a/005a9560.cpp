#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Item_005a9560 { uint8_t Unknown00[0x72]; uint8_t Flag72; };
struct Unknown_005a9560 {
    const uint32_t* Vptr00;
    uint8_t Unknown04[0x10];
    Item_005a9560** Slots14;
    uint8_t Unknown18[0x28];
    int32_t State40;
    uint8_t Unknown44[8];
    int32_t State4c;
};
#pragma pack(pop)

typedef void(__fastcall* Method0f4)(Unknown_005a9560* self, void* /*edx*/, Item_005a9560* item);

// Stores `item` into slot *index of the pointer table at +0x14. A null item returns 1. Otherwise, when the item's
// flag byte at +0x72 is set and the object is in state 1 (+0x4c) or 2 (+0x40), calls the virtual method at
// vtable +0xf4 with the item. Returns 0.
// FUNCTION: GP4 0x005a9560
int32_t __fastcall FUN_005a9560(Unknown_005a9560* self, void* /*edx*/, Item_005a9560* item, int32_t /*unused*/, const int16_t* index) {
    self->Slots14[*index] = item;
    if (item == nullptr) {
        return 1;
    }
    if (item->Flag72 != 0 && (self->State4c == 1 || self->State40 == 2)) {
        reinterpret_cast<Method0f4>(self->Vptr00[0xf4 / 4])(self, nullptr, item);
    }
    return 0;
}
GP4_IMPL(0x005a9560, FUN_005a9560, "thiscall(ptr[0x100]:{0:ptr[0x100]:{0xf4:u32{=0x0045a1c0}}, 0x14:ptr[0x40]:bytes, 0x40:i32{0,1,2,3}, 0x4c:i32{0,1,2,3}}, nptr[0x80]:{0x72:u8{0,1}}, i32, ptr[8]:i16[0..15]) -> i32")

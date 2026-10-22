#include <gp4/gp4.h>

typedef void(__thiscall* Slot0_004d2f40)(void* self);

#pragma pack(push, 1)
struct Unknown_004d2f40 {
    uint8_t Unknown00[0x9c];
    uint8_t* Holder9c;
    uint8_t* HolderA0;
};
#pragma pack(pop)

// Calls the first virtual method of the polymorphic sub-object embedded at +0xc of each of the
// two holder objects at +0x9c and +0xa0, then returns 0.
static inline void CallSubObjectSlot0(uint8_t* holder) {
    void* sub = holder + 0xc;
    void** vtable = *reinterpret_cast<void***>(sub);
    reinterpret_cast<Slot0_004d2f40>(vtable[0])(sub);
}

// FUNCTION: GP4 0x004d2f40
int32_t __fastcall FUN_004d2f40(Unknown_004d2f40* self, void* /*edx*/) {
    CallSubObjectSlot0(self->Holder9c);
    CallSubObjectSlot0(self->HolderA0);
    return 0;
}
GP4_IMPL(0x004d2f40, FUN_004d2f40,
         "thiscall(ptr[0xa8]:{0x9c:ptr[0x40]:{0xc:ptr[0x40]:{0:u32{=0x004c6540}}}, 0xa0:ptr[0x40]:{0xc:ptr[0x40]:{0:u32{=0x004c6540}}}}) -> i32")

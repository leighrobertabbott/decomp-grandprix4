#include <gp4/gp4.h>

struct Unknown_005a87d0;

#pragma pack(push, 1)
struct Unknown_005a87d0_Vtbl {
    void* Unknown00;
    void (__thiscall* Slot01)(Unknown_005a87d0* self);
    char (__thiscall* Slot02)(Unknown_005a87d0* self);
    void* Unknown0c[0x4e];
    int (__thiscall* Slot81)(Unknown_005a87d0* self);
};
struct Unknown_005a87d0 {
    Unknown_005a87d0_Vtbl* Vtbl00;
};
#pragma pack(pop)

// Runs the empty hook 0x005ae990, asks the object (virtual slot 0x08) whether it is ready and
// returns 1 if not. Otherwise it runs the hook again, calls virtual slot 0x144, then virtual slot
// 0x04 and returns the result of slot 0x144.
// FUNCTION: GP4 0x005a87d0
int __fastcall FUN_005a87d0(Unknown_005a87d0* self, void* /*edx*/) {
    gp4::cdecl_<0x005ae990, void>();
    if (self->Vtbl00->Slot02(self) == 0) {
        return 1;
    }
    gp4::cdecl_<0x005ae990, void>();
    int result = self->Vtbl00->Slot81(self);
    self->Vtbl00->Slot01(self);
    return result;
}
GP4_IMPL(0x005a87d0, FUN_005a87d0, "thiscall(ptr[0x10]:{0:ptr[0x150]:{4:u32{=0x005ae990}, 8:u32{=0x004a60c0,0x004aa530}, 0x144:u32{=0x004a60c0,0x004aa530}}}) -> i32")

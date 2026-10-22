#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005b3970 { const uint32_t* Vptr00; };
struct Unknown_005b3970 { uint8_t Unknown00[0x260]; Object_005b3970** Objects260; };
#pragma pack(pop)

typedef int32_t(__stdcall* Method50)(Object_005b3970* self, int32_t second, int32_t third, int32_t fourth, int32_t last);

// Looks up the object in slot (index & 0xffff) of the pointer table at +0x260. No object: returns 0xbbf.
// Otherwise calls its stdcall virtual method at vtable +0x50 with (object, a, b, c, 1) and returns 1 when it
// returned nonzero, else 0.
// FUNCTION: GP4 0x005b3970
int32_t __fastcall FUN_005b3970(Unknown_005b3970* self, void* /*edx*/, uint32_t index, int32_t a, int32_t b, int32_t c) {
    Object_005b3970* object = self->Objects260[index & 0xffff];
    if (object == nullptr) {
        return 0xbbf;
    }
    Method50 method = reinterpret_cast<Method50>(object->Vptr00[0x50 / 4]);
    return method(object, a, b, c, 1) != 0;
}
GP4_IMPL(0x005b3970, FUN_005b3970, "thiscall(ptr[0x280]:{0x260:ptr[0x20]:{0*4/4:nptr[0x80]:{0:ptr[0x60]:{0x50:u32{=0x00448f80}}}}}, u32{=0,1,2,3,0x10000,0x10001,0x20003}, i32, i32, i32) -> i32")

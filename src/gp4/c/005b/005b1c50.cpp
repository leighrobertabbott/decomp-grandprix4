#include <gp4/gp4.h>

struct Unknown_005b1c50_Object;

#pragma pack(push, 1)
struct Unknown_005b1c50_Object { const uint32_t* Vptr00; };
struct Unknown_005b1c50 { uint8_t Unknown00[0xc]; Unknown_005b1c50_Object* Object0c; };
#pragma pack(pop)

// Asks the object at +0xc (virtual method at vtable +8, thiscall with (index, &out1, &out2)) for the entry
// `index` and stores (low word of out2) - 1 through the second stack argument; returns the method's result,
// or 1 when there is no object. eax at the call is the vtable pointer, as the original leaves it.
// FUNCTION: GP4 0x005b1c50
int32_t __fastcall FUN_005b1c50(Unknown_005b1c50* self, void* /*edx*/, int32_t index, int16_t* result) {
    Unknown_005b1c50_Object* object = self->Object0c;
    if (object == nullptr) {
        return 1;
    }
    int32_t out1 = 0;
    int32_t out2 = 0;
    const uint32_t vtable = reinterpret_cast<uint32_t>(object->Vptr00);
    const uint32_t method = object->Vptr00[2];
    gp4::Regs regs = {};
    regs.eax = vtable;
    regs.ecx = reinterpret_cast<uint32_t>(object);
    regs.edx = static_cast<uint32_t>(index);
    const uint32_t args[3] = {static_cast<uint32_t>(index), reinterpret_cast<uint32_t>(&out1),
                              reinterpret_cast<uint32_t>(&out2)};
    gp4::call_regs_stdcall(method, regs, args);
    *result = static_cast<int16_t>(out2 - 1);
    return static_cast<int32_t>(regs.eax);
}
GP4_IMPL(0x005b1c50, FUN_005b1c50, "thiscall(ptr[0x20]:{0xc:nptr[0x40]:{0:ptr[0x20]:{8:u32{=0x005bc1f0}}}}, i32, ptr[8]:bytes) -> i32")

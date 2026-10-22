#include <gp4/gp4.h>

struct Engine_004d4ec0;
typedef int(__thiscall* EngineClose_004d4ec0)(Engine_004d4ec0*);
typedef void*(__thiscall* EngineDelete_004d4ec0)(Engine_004d4ec0*, int);

#pragma pack(push, 1)
// The audio engine object held at +0x14c: only its vtable is used here.
struct Engine_004d4ec0 { uint32_t* Vtable00; };
struct Unknown_004d4ec0 {
    uint32_t Vptr00;
    uint8_t Unknown04[0x0a];
    uint8_t Byte0e;
    uint8_t Unknown0f[0x131 - 0x0f];
    uint8_t Flag131;
    uint8_t Unknown132[0x14c - 0x132];
    Engine_004d4ec0* Source14c;
};
#pragma pack(pop)

// Scalar deleting destructor: stores the vtable pointer 0x005f0f48 at +0. When the object at +0x14c is
// set it gets its byte (0x005a9d70) set from +0xe; if the flag byte at +0x131 is 1 it is closed
// (virtual slot 0x74) and then deleted (virtual slot 0x18 with 1). When bit 0 of flags is set the
// object itself is freed with 0x00446a60. Returns this.
// FUNCTION: GP4 0x004d4ec0
Unknown_004d4ec0* __fastcall FUN_004d4ec0(Unknown_004d4ec0* self, void* /*edx*/, uint32_t flags) {
    self->Vptr00 = 0x005f0f48;
    if (self->Source14c != nullptr) {
        gp4::thiscall_<0x005a9d70, int>(self->Source14c, static_cast<int>(self->Byte0e));
        if (self->Flag131 == 1) {
            Engine_004d4ec0* engine = self->Source14c;
            reinterpret_cast<EngineClose_004d4ec0>(engine->Vtable00[0x74 / 4])(engine);
            engine = self->Source14c;
            if (engine != nullptr) {
                reinterpret_cast<EngineDelete_004d4ec0>(engine->Vtable00[0x18 / 4])(engine, 1);
            }
        }
    }
    if (flags & 1) {
        gp4::cdecl_<0x00446a60, int>(self);
    }
    return self;
}
GP4_IMPL(0x004d4ec0, FUN_004d4ec0, "thiscall(ptr[0x160]:{0xe:u8, 0x131:u8{0,1,2}, 0x14c:nptr[656]:layout=sndeng_005ac7a0}, u32) -> i32")

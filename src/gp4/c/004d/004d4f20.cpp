#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4f20 {
    uint32_t Vptr00;
    uint8_t Unknown04[4];
    uint32_t Field08;
    uint8_t Unknown0c[2];
    uint8_t Byte0e;
    uint8_t Unknown0f[0x131 - 0x0f];
    uint8_t Flag131;
    uint8_t Unknown132[0x14c - 0x132];
    void* Source14c;
};
#pragma pack(pop)

// Constructor: stores the vtable pointer 0x005f0f48 at +0, clears +8, sets the byte at +0x131 and
// clears the pointer at +0x14c, calls the empty routine 0x005ae990. When `source` is non-null it
// clears the byte at +0x131, stores source at +0x14c, copies source's byte getter 0x005a9d80 into
// the byte at +0xe and sets source's byte (0x005a9d70) to 0. Returns this.
// FUNCTION: GP4 0x004d4f20
Unknown_004d4f20* __fastcall FUN_004d4f20(Unknown_004d4f20* self, void* /*edx*/, void* source) {
    self->Vptr00 = 0x005f0f48;
    self->Field08 = 0;
    self->Flag131 = 1;
    self->Source14c = nullptr;
    gp4::cdecl_<0x005ae990, void>();
    if (source != nullptr) {
        self->Flag131 = 0;
        self->Source14c = source;
        uint8_t value = gp4::thiscall_<0x005a9d80, uint8_t>(source);
        self->Byte0e = value;
        gp4::thiscall_<0x005a9d70, int>(source, 0);
    }
    return self;
}
GP4_IMPL(0x004d4f20, FUN_004d4f20, "thiscall(ptr[0x160]:bytes, nptr[0x78]:bytes) -> u32")

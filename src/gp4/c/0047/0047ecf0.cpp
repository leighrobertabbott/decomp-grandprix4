#include <gp4/gp4.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

#pragma pack(push, 1)
struct Unknown_0047ecf0 {
    int32_t Value00;
    int32_t Value04;
    uint8_t Flag08;
    uint8_t Unknown09[0x53];
    int32_t Value5c;
    void* Mesh60;
    void* FontManager64;
};
#pragma pack(pop)

// ScreenObjectManager entry initialiser: stores the first argument at +0 and the second at +4, the mesh
// pointer at +0x60 and the font manager pointer at +0x64. Unless the first argument is -1 it validates
// the pointers: a null mesh or a null font manager is reported through OutputDebugStringA and the
// entry stays inactive, otherwise +0x5c is cleared and the active flag at +8 is set. Returns this.
// FUNCTION: GP4 0x0047ecf0
Unknown_0047ecf0* __fastcall FUN_0047ecf0(Unknown_0047ecf0* self, void* /*edx*/, int32_t first, int32_t second,
                                          void* mesh, void* fontManager) {
    self->Value04 = second;
    self->Value00 = first;
    self->Mesh60 = mesh;
    self->FontManager64 = fontManager;
    if (first != -1) {
        if (mesh == nullptr) {
            GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x006435c8));
            return self;
        }
        if (fontManager == nullptr) {
            GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x00643588));
            return self;
        }
        self->Value5c = 0;
        self->Flag08 = 1;
    }
    return self;
}
GP4_IMPL(0x0047ecf0, FUN_0047ecf0, "thiscall(ptr[0x70]:bytes, i32{-1,0,1,2}, i32, u32{0,0x1000,0x2000}, u32{0,0x1000,0x2000}) -> u32")

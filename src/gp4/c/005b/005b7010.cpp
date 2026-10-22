#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005b7010 { const uint32_t* Vptr00; };
#pragma pack(pop)

typedef int32_t(__fastcall* Method60)(Object_005b7010* self, void* /*edx*/, int32_t handle, int32_t zero, int32_t one, int32_t last);

// When the flag at 0x00b47608 is set, walks the chain of object indices that starts at 0x00b471c8 (next
// indices at 0x00b471cc, 0 ends the chain) and, for each, calls virtual method +0x60 of the object in the
// table at 0x00b3d52c with the matching handle from 0x00b473c8 and (0, 1, 1). Then clears the flag.
// FUNCTION: GP4 0x005b7010
void __cdecl FUN_005b7010() {
    if (GP4_GLOBAL(uint8_t, 0x00b47608) != 0) {
        int32_t index = GP4_GLOBAL(int32_t, 0x00b471c8);
        if (index != 0) {
            int32_t offset = 0;
            do {
                Object_005b7010* object = GP4_ARRAY(Object_005b7010*, 0x00b3d52c)[index];
                int32_t handle = GP4_GLOBAL(int32_t, 0x00b473c8 + offset);
                Method60 method = reinterpret_cast<Method60>(object->Vptr00[0x60 / 4]);
                method(object, nullptr, handle, 0, 1, 1);
                index = GP4_GLOBAL(int32_t, 0x00b471cc + offset);
                offset += 4;
            } while (index != 0);
        }
        GP4_GLOBAL(uint8_t, 0x00b47608) = 0;
    }
}
GP4_IMPL(0x005b7010, FUN_005b7010, "cdecl() -> void; gfields={0x00b47608:u8{=0,1}, 0x00b471c8:u32{=0,1,2,3}, 0x00b471cc*6/4:u32{=0,1,2,3}, 0x00b473c8*4/4:u32{=0,0xff,1,0xfe}, 0x00b3d52c*4/4:ptr[0x200]:{0:ptr[0x100]:{0x60:u32{=0x005a9f70}}}}")

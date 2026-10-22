#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bcd00 { uint8_t Unknown00[0x10]; int32_t Slots10[1]; };
struct Unknown_005bba20_Object { const uint32_t* Vptr00; };
#pragma pack(pop)

typedef void (__fastcall *Unknown_005bba20_Slot3c)(Unknown_005bba20_Object* self, void* edx, uint8_t* out);

// When the flag byte for the object index DAT_00b4765c (table at 0x00b304d4) is set and the object with
// that index exists in the table of DAT_00b471c4 (FUN_005bcd00), asks it through virtual slot 0x3c for a
// boolean (written through a pointer to a local byte) and stores "the answer is false" into 0x009ed5b0.
// FUNCTION: GP4 0x005bba20
void __cdecl FUN_005bba20(void) {
    const int32_t index = GP4_GLOBAL(int32_t, 0x00b4765c);
    if (GP4_ARRAY(uint8_t, 0x00b304d4)[index] == 0) {
        return;
    }
    Unknown_005bba20_Object* object = reinterpret_cast<Unknown_005bba20_Object*>(
        gp4::thiscall_<0x005bcd00, int32_t>(GP4_GLOBAL(Unknown_005bcd00*, 0x00b471c4), index));
    if (object == nullptr) {
        return;
    }
    Unknown_005bba20_Slot3c method = reinterpret_cast<Unknown_005bba20_Slot3c>(object->Vptr00[0x3c / 4]);
    uint8_t answer;
    method(object, nullptr, &answer);
    GP4_GLOBAL(uint8_t, 0x009ed5b0) = (answer == 0);
}
GP4_IMPL(0x005bba20, FUN_005bba20, "cdecl() -> void; globals=fuzz:i32; gfields={0x00b471c4:ptr[0x100]:{0x10*8/4:nptr[0x40]:{0:ptr[0x50]:{0x3c:u32{=0x005cbd80}}, 0x18:u8{0,1}}}, 0x00b4765c:i32[0..7], 0x00b304d4*8/1:u8{0,1}, 0x009ed5b0:u8{0,1}}")

#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Sink_004d2f70 {
    virtual int32_t Slot0() = 0;
    virtual int32_t Slot1() = 0;
    virtual int32_t Slot2() = 0;
    virtual int32_t Slot3() = 0;
    virtual int32_t Slot4() = 0;
    virtual int32_t Select(uint16_t index, int32_t arg) = 0;
};
struct Unknown_004d2f70 {
    uint8_t Unknown00[0x9c];
    Sink_004d2f70* Sink9c;
    Sink_004d2f70* SinkA0;
};
#pragma pack(pop)

// Picks the sink object at +0x9c (index < 4) or +0xa0 (index >= 4) and, when it exists, forwards
// index & 3 and the second argument to its virtual method at vtable +0x14. The original leaves
// eax as the sink's result, or the untouched index dword when there is no sink; callers read it.
// FUNCTION: GP4 0x004d2f70
uint32_t __fastcall FUN_004d2f70(Unknown_004d2f70* self, void* /*edx*/, uint32_t index, int32_t arg) {
    Sink_004d2f70* sink = (uint8_t)index < 4 ? self->Sink9c : self->SinkA0;
    if (sink != 0) {
        return (uint32_t)sink->Select((uint16_t)(index & 3), arg);
    }
    return index;
}
GP4_IMPL(0x004d2f70, FUN_004d2f70, "thiscall(ptr[0xa8]:{0x9c:nptr[0x80]:{0:ptr[0x40]:{0x14:u32{=0x464140}}}, 0xa0:nptr[0x80]:{0:ptr[0x40]:{0x14:u32{=0x464140}}}}, u32{0,1,2,3,4,5,6,7,0xff,0x12345602,0x12345680}, i32) -> u32")

#include <gp4/gp4.h>

struct Sink_004cb850;
typedef void(__fastcall* SinkSlot2)(Sink_004cb850* sink, void* edx, void* block);

#pragma pack(push, 1)
struct Sink_004cb850 {
    const uint32_t* Vptr00;
};
struct Unknown_004cb850 {
    uint8_t Unknown00[0x17d4];
    Sink_004cb850* Sink17d4;
};
#pragma pack(pop)

// Forwards the block at this+0x16c4 to virtual slot 2 (vtable +8) of the sink object stored at
// +0x17d4, when one is set.
// FUNCTION: GP4 0x004cb850
void __fastcall FUN_004cb850(Unknown_004cb850* self, void* /*edx*/) {
    Sink_004cb850* sink = self->Sink17d4;
    if (sink != nullptr) {
        const uint32_t* vtable = sink->Vptr00;
        ((SinkSlot2)vtable[2])(sink, (void*)vtable, reinterpret_cast<uint8_t*>(self) + 0x16c4);
    }
}
GP4_IMPL(0x004cb850, FUN_004cb850, "thiscall(ptr[0x1800]:{0x17d4:nptr[0x40]:{0:u32{=0x005f0d14}, 4:ptr[0x280]:{0x234:u32{=0}}}}) -> void")

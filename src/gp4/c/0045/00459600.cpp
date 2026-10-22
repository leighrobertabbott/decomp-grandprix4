#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Entry_00459600 {
    uint8_t Unknown00[0x20];
    int32_t Value20;
    int32_t Key24;
    uint8_t Unknown28[0x18];
};
struct Record_00459600 {
    uint8_t Unknown00[0x170];
    uint32_t Count170;
    Entry_00459600* Entries174;
    uint8_t Unknown178[0x54];
};
struct Unknown_00459600 {
    uint8_t Unknown00[0x1ec];
    uint32_t Count1ec;
    Record_00459600* Records1f0;
};
#pragma pack(pop)

// Walks the 0x1cc-byte records at +0x1f0 (count at +0x1ec); in each record, every 0x40-byte entry
// of its array (count at +0x170, pointer at +0x174) whose key at +0x24 equals `key` gets `value`
// stored at +0x20.
// FUNCTION: GP4 0x00459600
void __fastcall FUN_00459600(Unknown_00459600* self, void* /*edx*/, int32_t key, int32_t value) {
    for (uint32_t i = 0; i < self->Count1ec; i++) {
        Record_00459600* record = &self->Records1f0[i];
        for (uint32_t n = record->Count170; n != 0; n--) {
            Entry_00459600* entry = &record->Entries174[n - 1];
            if (entry->Key24 == key) {
                entry->Value20 = value;
            }
        }
    }
}
GP4_IMPL(0x00459600, FUN_00459600, "thiscall(ptr[0x200]:{0x1ec:u32[0..3], 0x1f0:ptr[0x5b0]:{0x170*3/0x1cc:u32[0..3], 0x174*3/0x1cc:ptr[0xf0]:{0x24*3/0x40:i32{=0,1,2}}}}, i32{=0,1,2}, i32) -> void")

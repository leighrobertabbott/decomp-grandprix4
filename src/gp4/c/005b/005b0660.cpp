#include <gp4/gp4.h>

#pragma pack(push, 1)
// The three name tables at +0x24 (allocated by the mixer constructor 0x005ae9c0).
struct MixNames_005b0660 {
    char** Sliders;     // +0, [+8] entries
    char** Groups;      // +4, [+0xc] entries
    char** Presets;     // +8, [+0x14] entries
};
struct MixFile_005b0660 {
    const void* Vptr00;
    int32_t Unknown04;
    int32_t NumFaders08;
    int32_t NumGroups0c;
    int32_t Unknown10;
    int32_t NumPresets14;
    uint8_t Unknown18[0xc];
    MixNames_005b0660* Names24;
    uint8_t Unknown28[8];
    void* File30;   // CRT FILE*
};
#pragma pack(pop)

static inline void WriteTag_005b0660(void* file, uint32_t tag) {
    gp4::cdecl_<0x004d47b0, uint32_t>(file, GP4_ARRAY(const char, tag), 4);
}

// Writes one 32-byte, zero-padded name record (names of 32 characters or more are cut to 31).
static inline void WriteName_005b0660(void* file, const char* src) {
    char name[0x20];
    for (int i = 0; i < 0x20; ++i) name[i] = 0;
    if (src != nullptr) {
        int len = 0;
        while (src[len] != 0) ++len;
        if (len < 0x20) {
            int i = 0;
            while ((name[i] = src[i]) != 0) ++i;
        } else {
            for (int i = 0; i < 0x1f; ++i) name[i] = src[i];
            name[0x1f] = 0;
        }
    }
    gp4::cdecl_<0x004d47b0, uint32_t>(file, name, 0x20);
}

// Writes the binary mixer name chunk: "NAME", then "SNAM" + one record per slider, "GNAM" + one per group
// and "PNAM" + one per preset (the ids are the strings the original writes).
// FUNCTION: GP4 0x005b0660
void __fastcall FUN_005b0660(MixFile_005b0660* self, void* /*edx*/) {
    WriteTag_005b0660(self->File30, 0x0063f1c8);   // "NAME"
    WriteTag_005b0660(self->File30, 0x009ed504);   // "SNAM"
    for (int32_t i = 0; i < self->NumFaders08; ++i) WriteName_005b0660(self->File30, self->Names24->Sliders[i]);
    WriteTag_005b0660(self->File30, 0x009ed4fc);   // "GNAM"
    for (int32_t i = 0; i < self->NumGroups0c; ++i) WriteName_005b0660(self->File30, self->Names24->Groups[i]);
    WriteTag_005b0660(self->File30, 0x009ed4f4);   // "PNAM"
    for (int32_t i = 0; i < self->NumPresets14; ++i) WriteName_005b0660(self->File30, self->Names24->Presets[i]);
}
GP4_IMPL(0x005b0660, FUN_005b0660,
         "thiscall(ptr[0x40]:{*bytes, 8:i32[-1..8], 0xc:i32[-1..8], 0x14:i32[-1..8], 0x24:ptr[0x10]:{0:ptr[0x20]:{0*8/4:nptr[0x40]:{*bytes, 0x10:u8{0,0x41}, 0x3f:u8{=0}}}, 4:ptr[0x20]:{0*8/4:nptr[0x40]:{*bytes, 0x10:u8{0,0x41}, 0x3f:u8{=0}}}, 8:ptr[0x20]:{0*8/4:nptr[0x40]:{*bytes, 0x10:u8{0,0x41}, 0x3f:u8{=0}}}}, 0x30:ptr[0x20]:{0:ptr[0x400]:bytes, 4:i32{=0x400}, 0xc:u32{=0x0a}, 0x10:u32{=0}, 0x18:i32{=0x1000}}}) -> void")

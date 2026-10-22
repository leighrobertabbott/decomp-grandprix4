#include <gp4/gp4.h>

typedef void* (__stdcall *GetProcessHeapFn)();
typedef void* (__stdcall *HeapAllocFn)(void* heap, uint32_t flags, uint32_t bytes);

#pragma pack(push, 1)
struct Rec48_004ea5a0 {
    uint8_t Unknown00;
    uint8_t Unknown01;
    uint8_t Unknown02;
    uint8_t Unknown03;
    uint8_t Unknown04;
    uint8_t Unknown05[0x43];
};
struct Unknown_004ea5a0 {
    uint32_t Unknown00;
    int32_t Count04;
    uint8_t Unknown08[0x40];
    int32_t* Array48;
    uint8_t Unknown4C[0x26b0 - 0x4c];
    Rec48_004ea5a0 Recs26B0[0x15f];   // ends at +0x8968
    int32_t* Array8968;
    int32_t* Array896C;
    int32_t* Array8970;
    int32_t* Array8974;
    int32_t* Array8978;
    int32_t* Array897C;
    int32_t* Array8980;
    uint8_t Unknown8984[0x89c8 - 0x8984];
    int32_t* Array89C8;
    int32_t* Array89CC;
    int32_t* Array89D0;
    int32_t* Array89D4;
    uint8_t Unknown89D8[8];
    int32_t Unknown89E0;
    int32_t Unknown89E4;
};
#pragma pack(pop)

static inline int32_t* Alloc_004ea5a0(int32_t count) {
    return (int32_t*)GP4_IAT(HeapAllocFn, 0x005f02a8)(GP4_IAT(GetProcessHeapFn, 0x005f02a4)(), 0u,
                                                       (uint32_t)(count << 2));
}

// Sets the element count at +4, allocates twelve count-dword arrays on the process heap,
// clears them (+0x8978 to -1), clears bytes 0/1/2/4 of the 0x15f 0x48-byte records at
// +0x26b0 and zeroes +0x89e0/+0x89e4.
// FUNCTION: GP4 0x004ea5a0
void __fastcall FUN_004ea5a0(Unknown_004ea5a0* self, void* /*edx*/, int32_t count) {
    self->Count04 = count;
    self->Array8974 = Alloc_004ea5a0(count);
    self->Array897C = Alloc_004ea5a0(self->Count04);
    self->Array8980 = Alloc_004ea5a0(self->Count04);
    self->Array8968 = Alloc_004ea5a0(self->Count04);
    self->Array896C = Alloc_004ea5a0(self->Count04);
    self->Array8970 = Alloc_004ea5a0(self->Count04);
    self->Array48 = Alloc_004ea5a0(self->Count04);
    self->Array8978 = Alloc_004ea5a0(self->Count04);
    self->Array89CC = Alloc_004ea5a0(self->Count04);
    self->Array89C8 = Alloc_004ea5a0(self->Count04);
    self->Array89D4 = Alloc_004ea5a0(self->Count04);
    self->Array89D0 = Alloc_004ea5a0(self->Count04);
    for (int32_t i = 0; i < self->Count04; ++i) {
        self->Array89CC[i] = 0;
        self->Array89C8[i] = 0;
        self->Array897C[i] = 0;
        self->Array8980[i] = 0;
        self->Array48[i] = 0;
        self->Array8968[i] = 0;
        self->Array896C[i] = 0;
        self->Array8970[i] = 0;
        self->Array8978[i] = -1;
    }
    for (int i = 0; i < 0x15f; ++i) {
        Rec48_004ea5a0* r = &self->Recs26B0[i];
        r->Unknown00 = 0;
        r->Unknown01 = 0;
        r->Unknown02 = 0;
        r->Unknown04 = 0;
    }
    self->Unknown89E4 = 0;
    self->Unknown89E0 = 0;
}
GP4_IMPL(0x004ea5a0, FUN_004ea5a0, "thiscall(ptr[0x89f0]:bytes, i32[0..64]) -> void")

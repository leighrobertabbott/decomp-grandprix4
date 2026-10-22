#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/d3d8.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

#pragma pack(push, 1)
struct LineVertex_005bc300 { float X; float Y; float Z; uint32_t Color; };   // FVF 0x42 = XYZ | DIFFUSE

struct Unknown_005bc300 {
    uint32_t Unknown00;
    int32_t Field04;
    int32_t Field08;
    int32_t Field0c;
    int32_t Field10;
    int32_t Field14;
    int32_t Field18;
    uint32_t Color1c;
    IDirect3DVertexBuffer8* VertexBuffer;   // +0x20 ("Line VB")
    int32_t Mode24;
};
#pragma pack(pop)

// Releases the old line vertex buffer (unbinding stream 0 first), creates a new managed XYZ|DIFFUSE one sized
// for `count` lines and seeds it: one vertex (0, 0, 0.5) for mode <= 1, else two vertices offset by
// -(mode * k / width) and -(mode * k / height).
// FUNCTION: GP4 0x005bc300
int32_t __fastcall FUN_005bc300(Unknown_005bc300* self, void* /*edx*/, int32_t count, int32_t a2, int32_t a3,
                                uint32_t color, int32_t mode) {
    using namespace gp4::x87;
    self->Field0c = a2;
    self->Field08 = a3;
    self->Field10 = a3;
    self->Field04 = a2;
    if (self->VertexBuffer != nullptr) {
        GP4_D3D_DEVICE->SetStreamSource(0, nullptr, 0);
        if (self->VertexBuffer->Release() != 0)
            GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x009ed73c));
    }
    self->VertexBuffer = nullptr;
    self->Mode24 = mode;
    GP4_D3D_DEVICE->CreateVertexBuffer(mode > 1 ? (uint32_t)(count << 6) : (uint32_t)((count + 1) << 4), 8, 0x42, 1,
                                       &self->VertexBuffer);
    self->Field14 = 0;
    self->Field18 = 0;
    self->Color1c = color;
    uint8_t* data;
    if (mode > 1) {
        if (self->VertexBuffer->Lock(0, 0x40, &data, 0) < 0) return (int32_t)0x80004005;
        const Ext f = ext_i32(self->Mode24) * ext_f32(GP4_GLOBAL(float, 0x005fa0cc));
        LineVertex_005bc300 v[2];
        v[0].X = narrow_f32(fchs(f / ext_i32(GP4_GLOBAL(int32_t, 0x009ed5b8))));
        v[0].Y = 0.0f;
        v[0].Z = 0.5f;
        v[0].Color = color;
        v[1].X = 0.0f;
        v[1].Y = narrow_f32(fchs(f / ext_i32(GP4_GLOBAL(int32_t, 0x009ed5bc))));
        v[1].Z = 0.5f;
        v[1].Color = color;
        LineVertex_005bc300* out = (LineVertex_005bc300*)data;
        out[0] = v[0];
        out[1] = v[1];
        self->VertexBuffer->Unlock();
        return 0;
    }
    if (self->VertexBuffer->Lock(0, 0x10, &data, 0) < 0) return (int32_t)0x80004005;
    LineVertex_005bc300* out = (LineVertex_005bc300*)data;
    out->X = 0.0f;
    out->Y = 0.0f;
    out->Z = 0.5f;
    out->Color = color;
    self->VertexBuffer->Unlock();
    return 0;
}
GP4_IMPL(0x005bc300, FUN_005bc300,
         "thiscall(ptr[0x30]:{*bytes, 0x20:nptr[16]:com=IDirect3DVertexBuffer8}, i32[0..64], i32, i32, u32, i32[-2..5]) -> i32; globals=layout=d3ddev_00b2e808; gfields={0x009ed5b8:i32, 0x009ed5bc:i32}; comfail=off:IDirect3DDevice8::CreateVertexBuffer")

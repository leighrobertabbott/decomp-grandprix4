#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
// 0xa0-byte per-car record of the renderer object (array at [[0xa539c0]+0x48]).
struct CarRec_004dea30 {
    uint8_t Unknown00[0x68];
    uint32_t X68;
    float Unknown6C;
    uint32_t Z70;
    uint8_t Unknown74[0xc];
    float Angle80;
    uint8_t Unknown84[0x1c];
};
struct Renderer_004dea30 {
    uint8_t Unknown00[0x48];
    CarRec_004dea30* Cars48;
};
struct Vec2_004dea30 { uint32_t X; uint32_t Z; };
struct Vec2f_004dea30 { float X; float Z; };
struct Vec3_004dea30 { float X; float Y; float Z; };
struct Matrix_004dea30 { float m[4][4]; };
struct Unknown_004dea30 {
    uint8_t Unknown00[0x7c];
    int32_t ViewCount7C;
};
#pragma pack(pop)

static inline float f32_of(uint32_t bits) { return *(const float*)&bits; }

// Car `car`'s ground position and heading direction. With [0xaa1b54] set and the byte
// [[0xa539bc]] set they come from the tables 0x662380/0x662440; otherwise, for a view
// index below +0x7c, the car's (x, z) offset from view position [0xaa0820][view] and its
// heading (sin/cos of -angle) are transformed by the inverse of matrix [0xaa1ae8][view]
// (0x55c8fa, D3DXMatrixInverse-shaped: out, null determinant, in); else the raw world
// position and heading are returned. The eax the original leaves (the last table dword,
// car*0xa0, or `dir`) is returned because the caller's gate checks eax.
// FUNCTION: GP4 0x004dea30
uint32_t __fastcall FUN_004dea30(Unknown_004dea30* self, void* /*edx*/, int view, int car,
                                 Vec2_004dea30* pos, Vec2f_004dea30* dir) {
    using namespace gp4::x87;
    if (GP4_GLOBAL(uint8_t, 0x00aa1b54) != 0) {
        if (*GP4_GLOBAL(const uint8_t*, 0x00a539bc) != 0) {
            Vec2_004dea30* d = (Vec2_004dea30*)dir;
            pos->X = GP4_ARRAY(Vec2_004dea30, 0x00662380)[car].X;
            pos->Z = GP4_ARRAY(Vec2_004dea30, 0x00662380)[car].Z;
            d->X = GP4_ARRAY(Vec2_004dea30, 0x00662440)[car].X;
            uint32_t z = GP4_ARRAY(Vec2_004dea30, 0x00662440)[car].Z;
            d->Z = z;
            return z;
        }
    } else if (view < self->ViewCount7C) {
        const CarRec_004dea30* rec = &GP4_GLOBAL(Renderer_004dea30*, 0x00a539c0)->Cars48[car];
        const Vec3_004dea30* eye = &GP4_GLOBAL(Vec3_004dea30*, 0x00aa0820)[view];
        float dx = narrow_f32(ext_f32(f32_of(rec->X68)) - ext_f32(eye->X));
        float dz = narrow_f32(ext_f32(f32_of(rec->Z70)) - ext_f32(eye->Z));
        Matrix_004dea30 inv;
        gp4::stdcall_<0x0055c8fa, Matrix_004dea30*>(&inv, (float*)nullptr,
                                                    GP4_GLOBAL(Matrix_004dea30*, 0x00aa1ae8) + view);
        Vec2f_004dea30* p = (Vec2f_004dea30*)pos;
        p->X = narrow_f32(ext_f32(inv.m[2][0]) * ext_f32(dz) + ext_f32(inv.m[0][0]) * ext_f32(dx));
        p->Z = narrow_f32(ext_f32(inv.m[2][2]) * ext_f32(dz) + ext_f32(inv.m[0][2]) * ext_f32(dx));
        const CarRec_004dea30* rec2 = &GP4_GLOBAL(Renderer_004dea30*, 0x00a539c0)->Cars48[car];
        Ext a = fchs(ext_f32(rec2->Angle80));
        Ext ns = fchs(fsin(a));
        Ext c = fcos(a);
        dir->X = narrow_f32(ext_f32(inv.m[2][0]) * c + ext_f32(inv.m[0][0]) * ns);
        dir->Z = narrow_f32(ext_f32(inv.m[2][2]) * c + ext_f32(inv.m[0][2]) * ns);
        return (uint32_t)dir;
    }
    uint32_t off = (uint32_t)car * 0xa0;
    const CarRec_004dea30* rec = &GP4_GLOBAL(Renderer_004dea30*, 0x00a539c0)->Cars48[car];
    pos->X = rec->X68;
    pos->Z = GP4_GLOBAL(Renderer_004dea30*, 0x00a539c0)->Cars48[car].Z70;
    dir->X = narrow_f32(fchs(fsin(fchs(ext_f32(GP4_GLOBAL(Renderer_004dea30*, 0x00a539c0)->Cars48[car].Angle80)))));
    dir->Z = narrow_f32(fcos(fchs(ext_f32(GP4_GLOBAL(Renderer_004dea30*, 0x00a539c0)->Cars48[car].Angle80))));
    return off;
}
GP4_IMPL(0x004dea30, FUN_004dea30, "thiscall(ptr[0x80]:{0x7c:i32[0..4]}, i32[0..3], i32[0..21], ptr[8]:bytes, ptr[8]:bytes) -> u32; gfields={0x00aa1b54:u8{=0,1}, 0x00a539bc:ptr[0x10]:{0:u8{=0,1}}, 0x00a539c0:ptr[0x70]:{0x48:ptr[0xdc0]:f32}, 0x00aa0820:ptr[0x30]:f32, 0x00aa1ae8:ptr[0x100]:f32, 0x00662380*48/4:f32, 0x00662440*44/4:f32}")

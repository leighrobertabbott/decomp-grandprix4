#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Vec3_00492c10 { float X, Y, Z; };
struct Matrix_00492c10 { float M[4][4]; };
#pragma pack(pop)

static inline void Identity_00492c10(Matrix_00492c10* m) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m->M[i][j] = (i == j) ? 1.0f : 0.0f;
}

// Builds a projection matrix (D3DX perspective FOV, fov 2*atan(0x5f09f0), aspect 1, near 1,
// far 100000) and a view matrix looking from pos + (0, 0x5f0770, 0) down to pos, with the up
// vector the rotation's forward axis (0,0,1 transformed as a normal) flattened onto the XZ plane
// and scaled to length 0x5f0428.
// FUNCTION: GP4 0x00492c10
void __fastcall FUN_00492c10(void* self, void* /*edx*/, Vec3_00492c10 pos, Matrix_00492c10 rot,
                             Matrix_00492c10* proj, Matrix_00492c10* view) {
    using namespace gp4::x87;
    (void)self;
    Vec3_00492c10 eye;
    eye.X = pos.X;
    eye.Y = narrow_f32(ext_f32(pos.Y) + ext_f32(GP4_GLOBAL(float, 0x005f0770)));
    eye.Z = pos.Z;
    const Ext half = fpatan(ext(GP4_GLOBAL(double, 0x005f09f0)), ext_one());
    Identity_00492c10(proj);
    Identity_00492c10(view);
    Vec3_00492c10 at = pos;
    Vec3_00492c10 fwd;
    fwd.X = 0.0f;
    fwd.Y = 0.0f;
    fwd.Z = 1.0f;
    gp4::stdcall_<0x0055d5e9, Matrix_00492c10*>(proj, narrow_f32(half + half), 1.0f, 1.0f, 100000.0f);
    rot.M[3][0] = 0.0f;
    rot.M[3][1] = 0.0f;
    rot.M[3][2] = 0.0f;
    Vec3_00492c10 dir;
    gp4::stdcall_<0x0055bd51, Vec3_00492c10*>(&dir, &fwd, &rot);
    const Ext k = ext_f32(GP4_GLOBAL(float, 0x005f0428)) /
                  fsqrt(ext_f32(dir.Z) * ext_f32(dir.Z) + ext_f32(dir.X) * ext_f32(dir.X));
    Vec3_00492c10 up;
    up.X = narrow_f32(ext_f32(dir.X) * k);
    up.Y = 0.0f;
    up.Z = narrow_f32(ext_f32(dir.Z) * k);
    gp4::stdcall_<0x0055d435, Matrix_00492c10*>(view, &eye, &at, &up);
}
GP4_IMPL(0x00492c10, FUN_00492c10, "thiscall(ptr[16]:bytes, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, f32, ptr[0x48]:f32, ptr[0x48]:f32) -> void")

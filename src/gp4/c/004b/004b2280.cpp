#include <gp4/gp4.h>
#include <gp4/d3d8.h>

// Direct3D device (second device global 0x00a6c0fc): SetRenderState(0x17 = D3DRS_ZFUNC, 4 = D3DCMP_LESSEQUAL).
// FUNCTION: GP4 0x004b2280
void __cdecl FUN_004b2280() {
    GP4_GLOBAL(IDirect3DDevice8*, 0x00a6c0fc)->SetRenderState(0x17, 4);
}
GP4_IMPL(0x004b2280, FUN_004b2280, "cdecl() -> void; globals=layout=d3ddev_00a6c0fc")

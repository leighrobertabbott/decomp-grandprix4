#include <gp4/gp4.h>

// Forwards to 0x00466490 with this = [0x009fb350] and the addresses of the two buffer cells
// 0x009fb3f4 and 0x009fb3e8.
// FUNCTION: GP4 0x0045ed80
void __cdecl FUN_0045ed80() {
    gp4::thiscall_<0x00466490, void>(GP4_GLOBAL(void*, 0x009fb350), reinterpret_cast<void*>(0x009fb3f4),
                                     reinterpret_cast<void*>(0x009fb3e8));
}
GP4_IMPL(0x0045ed80, FUN_0045ed80,
         "cdecl() -> void; gfields={0x009fb350:ptr[308]:layout=vbo_00466490, 0x009fb3f4:ptr[16]:com=IDirect3DVertexBuffer8, 0x009fb3e8:ptr[16]:com=IDirect3DIndexBuffer8}")

#include <gp4/gp4.h>
#include <gp4/d3d8.h>

typedef void*(__stdcall* GlobalFree_t)(void* mem);
typedef void*(__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

static inline void ReleaseAndClear_004aba40(IUnknown_** slot) {
    IUnknown_* obj = *slot;
    if (obj != nullptr) {
        obj->Release();
        *slot = nullptr;
    }
}

// Direct3D teardown on the second device 0x00a6c0fc: unbinds texture stages 0 and 1, clears
// 0x6f3c1d..0x6f3c21, releases every live entry of the 0x700-slot object table 0xa6c600 (entries
// -1 are free; slots 0x325..0x329, 0x42d..0x43c, 0x46b and 0x46c are left alone) and marks it free
// when Release returns 0, releases and clears the object globals 0xa66288..0xa66298, 0xa64c08,
// the two 13-entry tables 0xa67900 / 0xa6c5c8 and 0xa70a30, resets 0xa70988/0xa7098c through
// their setters, frees the global block 0xa709e4, clears the bytes 0xa64c0c / 0xa662ac, sets
// D3DRS_ZFUNC (0x4b22a0) and HeapFrees the 16 blocks of 0xa6c588.
// FUNCTION: GP4 0x004aba40
void __cdecl FUN_004aba40() {
    GP4_GLOBAL(IDirect3DDevice8*, 0x00a6c0fc)->SetTexture(0, nullptr);
    GP4_GLOBAL(IDirect3DDevice8*, 0x00a6c0fc)->SetTexture(1, nullptr);
    GP4_GLOBAL(uint32_t, 0x006f3c1e) = 0;
    GP4_GLOBAL(uint32_t, 0x006f3c1d) = 0;
    gp4::cdecl_<0x004a60c0, int>();
    for (IUnknown_** slot = (IUnknown_**)0x00a6c600; (int)slot < 0x00a6e200; slot++) {
        if (slot == (IUnknown_**)0x00a6d7ac || slot == (IUnknown_**)0x00a6d7b0) continue;
        if ((int)slot >= 0x00a6d6b4 && (int)slot <= 0x00a6d6f0) continue;
        if ((int)slot >= 0x00a6d294 && (int)slot <= 0x00a6d2a4) continue;
        IUnknown_* obj = *slot;
        if (obj == (IUnknown_*)-1) continue;
        if (obj->Release() == 0) *slot = (IUnknown_*)-1;
    }
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a66288));
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a6628c));
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a66290));
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a66294));
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a64c08));
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a66298));
    for (int i = 0; i < 13; i++) {
        ReleaseAndClear_004aba40(&GP4_ARRAY(IUnknown_*, 0x00a67900)[i]);
        ReleaseAndClear_004aba40(&GP4_ARRAY(IUnknown_*, 0x00a6c5c8)[i]);
    }
    gp4::cdecl_<0x004a5f70, int>(0);
    gp4::cdecl_<0x004a5f80, int>(0);
    ReleaseAndClear_004aba40(&GP4_GLOBAL(IUnknown_*, 0x00a70a30));
    void* block = GP4_GLOBAL(void*, 0x00a709e4);
    if (block != nullptr) {
        GP4_IAT(GlobalFree_t, 0x005f0244)(block);
        GP4_GLOBAL(void*, 0x00a709e4) = nullptr;
    }
    GP4_GLOBAL(uint8_t, 0x00a64c0c) = 0;
    GP4_GLOBAL(uint8_t, 0x00a662ac) = 0;
    gp4::cdecl_<0x004b22a0, void>();
    HeapFree_t heapFree = GP4_IAT(HeapFree_t, 0x005f02a0);
    GetProcessHeap_t getProcessHeap = GP4_IAT(GetProcessHeap_t, 0x005f02a4);
    for (void** p = (void**)0x00a6c588; (int)p < 0x00a6c5c8; p++) {
        if (*p != nullptr) heapFree(getProcessHeap(), 0, *p);
    }
}
GP4_IMPL(0x004aba40, FUN_004aba40, "cdecl() -> void; gfields={0x00a6c0fc:ptr[16]:com=IDirect3DDevice8, 0x00a6c600*1792/4:i32{=-1}, 0x00a6c600*64/4:ptr[16]:com=IDirect3DTexture8, 0x00a6d280*16/4:ptr[16]:com=IDirect3DTexture8, 0x00a6d6a8*24/4:ptr[16]:com=IDirect3DTexture8, 0x00a6d7a4*20/4:ptr[16]:com=IDirect3DTexture8, 0x00a6e1c0*16/4:ptr[16]:com=IDirect3DTexture8, 0x00a66288*5/4:nptr[16]:com=IDirect3DTexture8, 0x00a64c08:nptr[16]:com=IDirect3DTexture8, 0x00a67900*13/4:nptr[16]:com=IDirect3DTexture8, 0x00a6c5c8*13/4:nptr[16]:com=IDirect3DTexture8, 0x00a70a30:nptr[16]:com=IDirect3DTexture8, 0x00a709e4:u32{=0,0x00150000,0x12345678}, 0x00a6c588*16/4:nptr[0x40]:bytes}")

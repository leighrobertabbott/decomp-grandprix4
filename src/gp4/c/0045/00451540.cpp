#include <gp4/gp4.h>
#include <gp4/d3d8.h>

#pragma pack(push, 1)
// Texture slot of the renderer's texture manager: 0x4c bytes, the COM texture pointer first.
struct TextureSlot {
    IDirect3DTexture8* Texture;
    uint8_t Unknown04[0x48];
};
struct TextureManager {
    uint8_t Unknown00[0x38];
    TextureSlot Slots[1];
};
struct TextureOwner {
    uint8_t Unknown00[0x24];
    TextureManager* Manager;
};
#pragma pack(pop)

// Unlocks level `level` of texture slot `index` (IDirect3DTexture8::UnlockRect, vtable +0x44);
// true when the call succeeds. Index -1 means no texture: false.
// FUNCTION: GP4 0x00451540
bool __fastcall FUN_00451540(TextureOwner* self, void* /*edx*/, int index, uint32_t level) {
    if (index == -1) {
        return false;
    }
    IDirect3DTexture8* texture = self->Manager->Slots[index].Texture;
    HRESULT_ hr = texture->UnlockRect(level);
    return hr >= 0;
}
GP4_IMPL(0x00451540, FUN_00451540, "thiscall(ptr[0x40]:layout=texmgr_00451540, i32[-1..7], i32[0..3]) -> u8")

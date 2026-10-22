#include <gp4/gp4.h>
#include <gp4/d3d8.h>

#pragma pack(push, 1)
struct Pending_0044d440 {
    uint32_t Value00;
    uint8_t Key04;
    uint8_t Key05;
    uint8_t Unknown06[2];
};
struct Cache_0044d440 {
    uint8_t Unknown00[8];
    IDirect3DDevice8* Device08;
    uint8_t Unknown0C[0x28];
    uint32_t RenderState34[0x100];
    int8_t Dirty434[0x100];
    uint8_t Unknown534[0x40];
    uint8_t Unknown574[0x100];
    Pending_0044d440 PendingRs674[0x100];
    uint8_t PendingRsCountE74;
    uint8_t UnknownE75[3];
    uint32_t StageStateE78[0x100];
    int8_t Dirty1278[0x100];
    int32_t Slot1378[0x100];
    Pending_0044d440 PendingTss1778[0x100];
    uint8_t PendingTssCount1F78;
    uint8_t Unknown1F79[3];
    IDirect3DBaseTexture8* Texture1F7C[8];
    uint8_t TextureDirty1F9C[8];
    uint8_t TexturesDirty1FA4;
    uint8_t Unknown1FA5[3];
    const D3DMATERIAL8* Material1FA8;
    uint8_t MaterialDirty1FAC;
};
#pragma pack(pop)

// Flushes a Direct3D state cache to the device at +8: pending render states (+0x674, count
// +0xe74) go to SetRenderState when marked dirty or changed, dirty texture stages to
// SetTexture, a dirty material to SetMaterial, and pending texture stage states (+0x1778,
// count +0x1f78, key stage*32 + type) to SetTextureStageState; the queues are emptied.
// FUNCTION: GP4 0x0044d440
void __fastcall FUN_0044d440(Cache_0044d440* self, void* /*edx*/) {
    uint32_t i = 0;
    if (self->PendingRsCountE74 != 0) {
        do {
            Pending_0044d440* e = &self->PendingRs674[i];
            const uint32_t state = e->Key04;
            if (self->Dirty434[state] < 0 || e->Value00 != self->RenderState34[state]) {
                self->RenderState34[state] = e->Value00;
                self->Device08->SetRenderState(e->Key04, e->Value00);
            }
            i++;
            self->Unknown574[e->Key04] = 0xff;
            self->Dirty434[e->Key04] = 0;
        } while (i < self->PendingRsCountE74);
    }
    self->PendingRsCountE74 = 0;
    if (self->TexturesDirty1FA4 != 0) {
        self->TexturesDirty1FA4 = 0;
        for (uint32_t stage = 0; stage < 8; stage++) {
            if (self->TextureDirty1F9C[stage] & 1) {
                self->TextureDirty1F9C[stage] = 0;
                self->Device08->SetTexture(stage, self->Texture1F7C[stage]);
            }
        }
    }
    if (self->MaterialDirty1FAC & 1) {
        self->MaterialDirty1FAC = 0;
        self->Device08->SetMaterial(self->Material1FA8);
    }
    i = 0;
    if (self->PendingTssCount1F78 != 0) {
        do {
            Pending_0044d440* e = &self->PendingTss1778[i];
            const uint32_t key = (uint32_t)e->Key04 * 0x20 + e->Key05;
            if (self->Dirty1278[key] < 0 || e->Value00 != self->StageStateE78[key]) {
                self->StageStateE78[key] = e->Value00;
                self->Device08->SetTextureStageState(e->Key04, e->Key05, e->Value00);
            }
            self->Slot1378[(uint32_t)e->Key04 * 0x20 + e->Key05] = -1;
            i++;
            self->Dirty1278[(uint32_t)e->Key04 * 0x20 + e->Key05] = 0;
        } while (i < self->PendingTssCount1F78);
    }
    self->PendingTssCount1F78 = 0;
}
GP4_IMPL(0x0044d440, FUN_0044d440, "thiscall(ptr[0x1fb8]:{*bytes, 8:ptr[16]:com=IDirect3DDevice8, 0x34*256/4:u32[0..1], 0x434*256/1:i8[-1..1], 0x674*256/8:u32[0..1], 0x678*256/8:u8, 0xe74:u8{0,1,2,7,255}, 0xe78*256/4:u32[0..1], 0x1278*256/1:i8[-1..1], 0x1778*256/8:u32[0..1], 0x177c*256/8:u8[0..7], 0x177d*256/8:u8[0..31], 0x1f78:u8{0,1,2,7,255}, 0x1f7c*8/4:nptr[16]:com=IDirect3DTexture8, 0x1f9c*8/1:u8, 0x1fa4:u8{=0,1}, 0x1fa8:ptr[0x44]:f32, 0x1fac:u8}) -> void")

#include <gp4/gp4.h>
#include <gp4/x87.h>

typedef void* (__stdcall *GetProcessHeapFn)();
typedef void* (__stdcall *HeapAllocFn)(void* heap, uint32_t flags, uint32_t bytes);

#pragma pack(push, 1)
// 0x20-byte vertex: position, normal, diffuse, specular (D3DFVF order).
struct Vertex_00459a40 {
    float Pos[3];
    float Normal[3];
    uint32_t Diffuse;
    uint32_t Specular;
};
// 0x40 bytes of texture coordinates: 8 U values then 8 V values.
struct TexCoords_00459a40 {
    float U[8];
    float V[8];
};
// Heap-allocated (0x6c bytes) node of the vertex list.
struct VertexNode_00459a40 {
    Vertex_00459a40 Vertex;
    TexCoords_00459a40 Tex;
    int32_t Key60;
    int32_t Refs64;
    VertexNode_00459a40* Next68;
};
struct VertexList_00459a40 {
    uint32_t Unknown00;
    VertexNode_00459a40* First04;
    int32_t Count08;
    uint32_t Fvf0C;
};
#pragma pack(pop)

// Looks up a vertex in the list (fields compared according to the FVF flags at +0xc:
// XYZ and NORMAL within a squared-distance tolerance, DIFFUSE/SPECULAR exactly, then
// (fvf >> 8) & 0xf texture coordinate pairs). On a hit the node's reference count is
// bumped; otherwise a new node is appended. Returns the node's index in the list.
// FUNCTION: GP4 0x00459a40
int __fastcall FUN_00459a40(VertexList_00459a40* self, void* /*edx*/, const Vertex_00459a40* v,
                            const TexCoords_00459a40* tex, int key, int /*unused*/) {
    using namespace gp4::x87;
    VertexNode_00459a40** link = &self->First04;
    VertexNode_00459a40* node = self->First04;
    int index = 0;
    while (node != nullptr) {
        if (node->Key60 == key) {
            uint32_t fvf = self->Fvf0C;
            if (fvf & 2) {
                Ext dz = ext_f32(node->Vertex.Pos[2]) - ext_f32(v->Pos[2]);
                Ext dy = ext_f32(node->Vertex.Pos[1]) - ext_f32(v->Pos[1]);
                Ext dx = ext_f32(node->Vertex.Pos[0]) - ext_f32(v->Pos[0]);
                Ext d = dx * dx + dy * dy + dz * dz;
                if ((fcom(d, ext_f32(GP4_GLOBAL(float, 0x005f0420))) & 0x4100) == 0) goto next;
            }
            if (fvf & 0x10) {
                Ext dz = ext_f32(node->Vertex.Normal[2]) - ext_f32(v->Normal[2]);
                Ext dy = ext_f32(node->Vertex.Normal[1]) - ext_f32(v->Normal[1]);
                Ext dx = ext_f32(node->Vertex.Normal[0]) - ext_f32(v->Normal[0]);
                Ext d = dz * dz + dx * dx + dy * dy;
                if ((fcom(d, ext_f32(GP4_GLOBAL(float, 0x005f0420))) & 0x4100) == 0) goto next;
            }
            if ((fvf & 0x40) && node->Vertex.Diffuse != v->Diffuse) goto next;
            if ((fvf & 0x80) && node->Vertex.Specular != v->Specular) goto next;
            {
                uint32_t n = (fvf >> 8) & 0xf;
                while (n != 0) {
                    --n;
                    if (node->Tex.U[n] != tex->U[n]) goto next;
                    if (node->Tex.V[n] != tex->V[n]) goto next;
                }
            }
            node->Refs64 = node->Refs64 + 1;
            return index;
        }
    next:
        link = &node->Next68;
        ++index;
        node = node->Next68;
    }
    {
        VertexNode_00459a40* fresh = (VertexNode_00459a40*)GP4_IAT(HeapAllocFn, 0x005f02a8)(
            GP4_IAT(GetProcessHeapFn, 0x005f02a4)(), 0u, 0x6cu);
        *link = fresh;
        (*link)->Vertex = *v;
        (*link)->Tex = *tex;
        (*link)->Refs64 = 0;
        (*link)->Key60 = key;
        (*link)->Next68 = nullptr;
        self->Count08 = self->Count08 + 1;
        node = *link;
    }
    node->Refs64 = node->Refs64 + 1;
    return index;
}
GP4_IMPL(0x00459a40, FUN_00459a40, "thiscall(ptr[0x14]:{4:nptr[0x6c]:{0*24/4:u32{=0,0x3f800000}, 0x60:i32{=0,1}, 0x64:i32[0..100], 0x68:nptr[0x6c]:{0*24/4:u32{=0,0x3f800000}, 0x60:i32{=0,1}, 0x64:i32[0..100], 0x68:nptr[0x6c]:{0*24/4:u32{=0,0x3f800000}, 0x60:i32{=0,1}, 0x64:i32[0..100], 0x68:u32{=0}}}}, 8:i32[0..100], 0xc:u32[0..0x8ff]}, ptr[0x20]:{0*8/4:u32{=0,0x3f800000}}, ptr[0x40]:{0*16/4:u32{=0,0x3f800000}}, i32{=0,1}, i32) -> i32")

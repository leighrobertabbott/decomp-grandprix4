#include <gp4/gp4.h>

// Block behind the pointer global 0x006097b0: header position, record count at
// +0x18, results at +0x20..+0x37, then 0x14-byte records of x/y/z/kind.
struct Unknown_Record14 {
    int32_t X;
    int32_t Y;
    int32_t Z;
    int32_t Kind;
    int32_t Unknown10;
};

struct Unknown_Block {
    int32_t X;
    int32_t Y;
    int32_t Z;
    uint8_t Unknown0c[0x0c];
    int32_t Count;
    uint8_t Unknown1c[4];
    int32_t CenterX;
    int32_t CenterY;
    int32_t CenterZ;
    int32_t HalfX;
    int32_t HalfY;
    int32_t HalfZ;
    Unknown_Record14 Records[1];
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x00424625
void __cdecl FUN_00424625(gp4::Regs* r) {
    Unknown_Block* block = GP4_GLOBAL(Unknown_Block*, 0x006097b0);
    const Unknown_Record14* rec = block->Records;

    int32_t& minX = GP4_GLOBAL(int32_t, 0x00607a14);
    int32_t& maxX = GP4_GLOBAL(int32_t, 0x00607a18);
    int32_t& minY = GP4_GLOBAL(int32_t, 0x00607a1c);
    int32_t& maxY = GP4_GLOBAL(int32_t, 0x00607a20);
    int32_t& minZ = GP4_GLOBAL(int32_t, 0x00607a24);
    int32_t& maxZ = GP4_GLOBAL(int32_t, 0x00607a28);
    // half-extent tables indexed by 0 (kind 2) or 1 (kind 1)
    const int32_t* extX = GP4_ARRAY(const int32_t, 0x00607da8);
    const int32_t* extY = GP4_ARRAY(const int32_t, 0x00607db0);
    const int32_t* extZ = GP4_ARRAY(const int32_t, 0x00607db8);

    minX = 0x7fffffff;
    minY = 0x7fffffff;
    minZ = 0x7fffffff;
    maxX = (int32_t)0x80000000u;
    maxY = (int32_t)0x80000000u;
    maxZ = (int32_t)0x80000000u;

    int32_t i = 0;
    do {
        const int32_t kind = rec->Kind;
        if (kind == 1 || kind == 2) {
            const int idx = (kind == 1) ? 1 : 0;

            int32_t h = extX[idx];
            int32_t v = (int32_t)((uint32_t)rec->X - (uint32_t)block->X + (uint32_t)h);
            if (maxX < v) maxX = v;
            v = (int32_t)((uint32_t)v - (uint32_t)h - (uint32_t)h);
            if (minX > v) minX = v;

            h = extY[idx];
            v = (int32_t)((uint32_t)rec->Y - (uint32_t)block->Y + (uint32_t)h);
            if (maxY < v) maxY = v;
            v = (int32_t)((uint32_t)v - (uint32_t)h - (uint32_t)h);
            if (minY > v) minY = v;

            h = extZ[idx];
            v = (int32_t)((uint32_t)rec->Z - (uint32_t)block->Z + (uint32_t)h);
            if (maxZ < v) maxZ = v;
            v = (int32_t)((uint32_t)v - (uint32_t)h - (uint32_t)h);
            if (minZ > v) minZ = v;
        } else {
            int32_t v = (int32_t)((uint32_t)rec->X - (uint32_t)block->X);
            if (maxX < v) maxX = v;
            if (minX > v) minX = v;

            v = (int32_t)((uint32_t)rec->Y - (uint32_t)block->Y);
            if (maxY < v) maxY = v;
            if (minY > v) minY = v;

            v = (int32_t)((uint32_t)rec->Z - (uint32_t)block->Z);
            if (maxZ < v) maxZ = v;
            if (minZ > v) minZ = v;
        }
        rec++;
        i++;
    } while (i != block->Count);

    block->HalfX = (int32_t)((uint32_t)maxX - (uint32_t)minX) >> 1;
    block->CenterX = (int32_t)((uint32_t)maxX + (uint32_t)minX) >> 1;
    block->HalfY = (int32_t)((uint32_t)maxY - (uint32_t)minY) >> 1;
    block->CenterY = (int32_t)((uint32_t)maxY + (uint32_t)minY) >> 1;
    block->HalfZ = (int32_t)((uint32_t)maxZ - (uint32_t)minZ) >> 1;
    const uint32_t sumZ = (uint32_t)maxZ + (uint32_t)minZ;
    const int32_t centerZ = (int32_t)sumZ >> 1;
    block->CenterZ = centerZ;

    // pushal/popal restore every register; the flags of the last writer
    // (`sar eax, 1` of the Z sum) stay: CF = bit shifted out, OF = AF = 0.
    const uint32_t result = (uint32_t)centerZ;
    const uint32_t flags = (sumZ & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424625, FUN_00424625, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_00424625")

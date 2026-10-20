#include <gp4/gp4.h>

struct Unknown_0040ecc4_Car {
    uint8_t Unknown00[0x28];
    uint32_t Unknown28;
    uint32_t Unknown2C;
    uint32_t Unknown30;
    uint32_t Unknown34;
    uint8_t Unknown38[0x7c - 0x38];
    uint8_t CarNumber7C;
    uint8_t Unknown7D[0x162 - 0x7d];
    uint8_t Flags162;
};

// FUNCTION: GP4 0x0040ecc4
// pushal ... popal wrapper on the car record in esi: sets bit 4 of byte +0x162, stores 0x20000000
// (0x10000000 for +0x28) into the dwords +0x28..+0x34, and fills the entry of the car number
// ((byte +0x7c & 0x3f) - 1, at most 0x15) in the three 22-dword tables 0x00675970 / 0x006759c8 /
// 0x00675a20, then re-sorts the order list through 0x0051ef71 and sets byte 0x00852d70 to 0x80.
// popal restores every register; the flags are those 0x0051ef71 leaves.
void __cdecl FUN_0040ecc4(gp4::Regs* r) {
    Unknown_0040ecc4_Car* car = reinterpret_cast<Unknown_0040ecc4_Car*>(static_cast<uintptr_t>(r->esi));

    car->Flags162 = static_cast<uint8_t>(car->Flags162 | 0x10u);
    car->Unknown34 = 0x20000000u;
    car->Unknown2C = 0x20000000u;
    car->Unknown30 = 0x20000000u;
    car->Unknown28 = 0x10000000u;

    int32_t index = static_cast<int32_t>(car->CarNumber7C & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    GP4_ARRAY(uint32_t, 0x00675970)[index] = 0x20000000u;
    GP4_ARRAY(uint32_t, 0x006759c8)[index] = 0x10000000u;
    GP4_ARRAY(uint32_t, 0x00675a20)[index] = 0x10000000u;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x0051ef71, call);

    GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040ecc4, FUN_0040ecc4, "regs(esi:ptr[0x16a]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040ee7b+order_00675958")

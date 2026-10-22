#include <gp4/gp4.h>

#pragma pack(push, 1)
// 12-byte table entry whose first dword is the id; the table ends with id 0.
struct IdEntry {
    int32_t Id;
    uint8_t Unknown04[8];
};
#pragma pack(pop)

// Looks `id` up in a 12-byte-stride table terminated by id 0: 0 for id 0, the entry address when found,
// -1 when the table has no such id.
// FUNCTION: GP4 0x00503b30
IdEntry* __stdcall FUN_00503b30(int id, IdEntry* table) {
    if (id == 0) {
        return nullptr;
    }
    int32_t current = table->Id;
    if (current != 0) {
        while (id != current) {
            current = table[1].Id;
            table = table + 1;
            if (current == 0) {
                return reinterpret_cast<IdEntry*>(-1);
            }
        }
        if (table->Id != 0) {
            return table;
        }
    }
    return reinterpret_cast<IdEntry*>(-1);
}
GP4_IMPL(0x00503b30, FUN_00503b30, "stdcall(u32{=0,0,1,2,3}, ptr[0x100]:u32{=0,0,1,2,3}) -> u32")

#include <gp4/gp4.h>

#pragma pack(push, 1)
struct IdValue { uint32_t Id; int32_t Value; };
#pragma pack(pop)

// Looks `id` up in a table of {id, value} dword pairs that ends at the first pair whose id is zero.
// Returns the value of the matching pair, -1 when there is no match (or the table is empty), and 0 when
// the table pointer or the id is null.
// FUNCTION: GP4 0x004fd570
int __stdcall FUN_004fd570(const IdValue* table, uint32_t id) {
    if (table == nullptr || id == 0u) {
        return 0;
    }
    uint32_t current = table->Id;
    if (current != 0u) {
        while (id != current) {
            current = table[1].Id;
            table++;
            if (current == 0u) {
                return -1;
            }
        }
        if (table->Id != 0u) {
            return table->Value;
        }
    }
    return -1;
}
GP4_IMPL(0x004fd570, FUN_004fd570, "stdcall(nptr[0x100]:u32{=0,0,1,2,3}, u32{=0,1,2,3}) -> i32")

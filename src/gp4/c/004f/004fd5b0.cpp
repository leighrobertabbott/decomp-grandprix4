#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004fd5b0 { uint32_t Value; uint32_t Key; };
#pragma pack(pop)

// Looks up `key` in a table of {value, key} dword pairs that ends at the first pair whose value is
// zero. Returns the value of the matching pair, -1 when there is no match (or the table is empty), and
// 0 when the table pointer or the key is null.
// FUNCTION: GP4 0x004fd5b0
int __stdcall FUN_004fd5b0(const Unknown_004fd5b0* table, uint32_t key) {
    if (table == nullptr || key == 0u) {
        return 0;
    }
    if (table->Value != 0u) {
        while (key != table->Key) {
            table++;
            if (table->Value == 0u) {
                return -1;
            }
        }
        if (table->Value != 0u) {
            return static_cast<int>(table->Value);
        }
    }
    return -1;
}
GP4_IMPL(0x004fd5b0, FUN_004fd5b0, "stdcall(nptr[0x100]:u32{0,1,2,3}, u32{0,1,2,3}) -> i32")

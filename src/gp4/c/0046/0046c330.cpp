#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_46c330 {
    uint8_t Unknown00[4];
    int32_t Count04;
    uint8_t Unknown08[0x358 - 8];
    int32_t List358[1];
};
#pragma pack(pop)

// C-style variadic list: stores the int arguments that follow the object, up to (not including)
// the terminating -1, into the table at +0x358 and counts them at +0x04. The argument cursor
// walks the pushed stack arguments directly and the current value is the stack slot of the first
// one, as the original does.
// FUNCTION: GP4 0x0046c330
void __cdecl FUN_0046c330(Unknown_46c330* obj, int value, ...) {
    obj->Count04 = 0;
    if (value != -1) {
        const int* next = &value;
        do {
            ++next;
            obj->List358[obj->Count04] = value;
            value = *next;
            obj->Count04 = obj->Count04 + 1;
        } while (value != -1);
    }
}
GP4_IMPL(0x0046c330, FUN_0046c330, "cdecl(ptr[0x380]:bytes, i32{-1,3,5,7}, i32{-1,2,6,9}, i32{-1,4,8}, i32{-1,1}, i32{=-1}) -> void")

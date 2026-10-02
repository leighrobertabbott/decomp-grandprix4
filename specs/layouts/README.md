# Emulator input layouts

A spec argument `ptr[0x200]:layout=<name>` fills the buffer from
`<name>.json` instead of random bytes, so the original function sees plausible
data (finite floats, valid indices, real nested pointers) and its cases stay valid.

```json
{
  "size": 512,
  "fields": [
    {"off": 0,    "type": "f64", "lo": -3.15, "hi": 3.15},
    {"off": 8,    "type": "f32", "lo": 0,     "hi": 90},
    {"off": 16,   "type": "i32", "lo": 0,     "hi": 21},
    {"off": 20,   "type": "ptr", "size": 64,  "fill": "f32"}
  ]
}
```

Types: `i8 u8 i16 u16 i32 u32 f32 f64 ptr`. Unlisted bytes are zero. The lead
writes these alongside `include/gp4/types/*.h` when workers ask for a layout.

For fixed-address state, `; globals=layout=<name>` uses a layout with a
`globals` array. Each entry specifies `addr`, `type`, and optional `lo`/`hi`;
`count` and `stride` repeat a field across records. Addresses and strides may
be integers or hexadecimal strings. Only declared fields are varied, so a
byte discriminator can be tested without corrupting neighbouring pointers.
`search_0066d82c.json` records the fields read by `0x00409c13`.

Repeated `u8` fields may use `"pattern": "bit_scan", "bit": 128` to mix
independent bytes with all-clear and exactly-one-set-bit cases. Other bits
still vary. This exercises both early exits and an exhausted scan without
changing the function's memory model. The pattern cannot also use `lo`/`hi`.

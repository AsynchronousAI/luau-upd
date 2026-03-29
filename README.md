# Luau N-len vector mod
This is modded Luau implementing arbitrary length vectors for more than 3 dimensions:
- Supports up to 1024 dimensions
- Supports Native Luau
- This delivers 30x the performance in matrix multiplication (without Native Luau).
- Adds `vector.dim` function to count how many dimensions are in a vector.
- Vectors of size `<= 3` are represented the same, but larger vectors are represented by reference.

```luau
local v1 = vector.create(1,2,3,4,5)
local v2 = vector.create(5,4,3,2,1)

print(v1 * v2)
```

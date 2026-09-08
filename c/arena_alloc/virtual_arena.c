#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ALIGN_UP_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))

#define ARENA_BASE_POS (sizeof(mem_arena))
#define ARENA_ALIGN (sizeof(void*))

typedef struct {
    u64 reserve_size;
    u64 commit_size;

    u64 pos;
    u64 commit_pos;
} mem_arena;

mem_arena* arena_create(u64 reserve_size, u64 commit_size);
void arena_destroy(mem_arena* arena);
void* arena_push(mem_arena* arena, u64 size, bool nonzero);
void* arena_pop(mem_arena* arena, u64 size);
void* arena_pop_to(mem_arena* arena, u64 pos);
void* arena_clear(mem_arena* arena);

#define PUSH_STRUCT(arena, T) (T*)arena_push((arena), sizeof(T), false)
#define PUSH_STRUCT_NZ(arena, T) (T*)arena_push((arena), sizeof(T), true)
#define PUSH_ARRAY(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), false)
#define PUSH_ARRAY_NZ(arena, T, n) (T*)arena_push((arena), sizeof(T) * (n), true)

u32 plat_get_pagesize(void);

void* plat_mem_reserve(u64 size);            // Reserve bytes in virtual address space
bool plat_mem_commit(void* ptr, u64 size);   // Commit to that portion of virtual address space
bool plat_mem_decommit(void* ptr, u64 size);
bool plat_mem_release(void* ptr, u64 size);

int main(void) {
    // Nowadays, instead of pointers directly pointing to the actual memory in ram
    // they instead point to virtual addresses
    // The computer then uses a Memory Management Unit (MMU) to map virtual address space to
    // actual address space. This helps security and helps avoid fragmentation
    mem_arena* perm_arena = arena_create(GiB(1), MiB(1));

    while (1) {
        arena_push(perm_arena, MiB(16), false);
        getc(stdin);
    }

    arena_destroy(perm_arena);

    return 0;
}

mem_arena* arena_create(u64 reserve_size, u64 commit_size) {
    u32 page_size = plat_get_pagesize();

    reserve_size = ALIGN_UP_POW2(reserve_size, page_size);
    commit_size = ALIGN_UP_POW2(commit_size, page_size);

    mem_arena* arena = plat_mem_reserve(reserve_size);

    if (!plat_mem_commit(arena, commit_size)) { return NULL; }

    arena->reserve_size = reserve_size;
    arena->commit_size = commit_size;
    arena->pos = ARENA_BASE_POS;
    arena->commit_pos = commit_size;

    return arena;
}

void arena_destroy(mem_arena* arena) {
    plat_mem_release(arena, arena->reserve_size);
}

void* arena_push(mem_arena* arena, u64 size, bool nonzero) {
    // Align the memory first to the nearest sizeof(void*)
    // CPUs read memory in chunks--usually 4-byte or 8-byte blocks (words)
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);

    u64 new_pos = pos_aligned + size;

    // Check that the new_pos is within the reserved limits
    if (new_pos > arena->reserve_size) { return NULL; }

    // If within reserved limits but not enough commit size, make new commit
    if (new_pos > arena->commit_pos) {
        u64 new_commit_pos = new_pos;
        new_commit_pos += arena->commit_size - 1;
        new_commit_pos -= new_commit_pos % arena->commit_size;
        new_commit_pos = MIN(new_commit_pos, arena->reserve_size);

        u8* mem = (u8*)arena + arena->commit_pos;
        u64 commit_size = new_commit_pos - arena->commit_pos;

        // Increase the commit size (note that mem is the old commit position in the arena)
        if (!plat_mem_commit(mem, commit_size)) { return NULL; }

        arena->commit_pos = new_commit_pos;
    }

    arena->pos = new_pos;

    u8* out = (u8*)arena + pos_aligned;

    if (!nonzero) { memset(out, 0, size); }

    return out;
}

void* arena_pop(mem_arena* arena, u64 size) {
    size = MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}

void* arena_pop_to(mem_arena* arena, u64 pos) {
    u64 size = pos < arena->pos ? arena->pos - pos : 0;
    arena_pop(arena, size);
}

void* arena_clear(mem_arena* arena) {
    arena_pop_to(arena, ARENA_BASE_POS);
}

#ifdef _WIN32

#include <windows.h>

u32 plat_get_pagesize(void) {
    SYSTEM_INFO sys_info = { 0 };
    GetSystemInfo(&sys_info);

    return sys_info.dwPageSize;
}

void* plat_mem_reserve(u64 size) {
    // Tell the OS to set aside a contiguous block of virtual addresses for virtual use
    // No physical RAM or page file is consumed during this step
    // Ensures that as the arena grows, it remains perfectly contiguous in memory

    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

bool plat_mem_commit(void* ptr, u64 size) {
    // The OS allocates physical memory or pagefile space to back those virtual addresses
    // As the arena allocator's internal offset pointer moves forward and crosses a page boundary
    // it dynamically commits the next page

    void* ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return ret != NULL;
}

bool plat_mem_decommit(void* ptr, u64 size) {
    // Tell the OS to free up the physical RAM but keep the addresses reserved
    // The memory becomes inaccessible but nobody else can take that virtual address space

    void* ret = VirtualFree(ptr, 0, MEM_DECOMMIT);
    return ret != NULL;
}

bool plat_mem_release(void* ptr, u64 size) {
    // The virtual address space is freed and any remaining physical RAM backing is returned

    return VirtualFree(ptr, 0, MEM_RELEASE);
}

#endif
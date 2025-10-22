#ifndef KRUEGER_BASE_ARENA_C
#define KRUEGER_BASE_ARENA_C

//////////////
// NOTE: Arena

internal Arena
make_arena(u8 *mem, uxx res_size) {
  Arena result = {
    .res_size = res_size,
    .cmt_size = 0,
    .mem = mem,
  };
  return(result);
}

internal void *
arena_push(Arena *arena, uxx cmt_size) {
  assert((arena->cmt_size + cmt_size) <= arena->res_size);
  void *result = (void *)(arena->mem + arena->cmt_size);
  arena->cmt_size += cmt_size;
  return(result);
}

internal Temp
temp_begin(Arena *arena) {
  Temp result = {
    .arena = arena,
    .cmt_size = arena->cmt_size,
  };
  return(result);
}

internal void
temp_end(Temp temp) {
  Arena *arena = temp.arena;
  arena->cmt_size = temp.cmt_size;
}

#endif // KRUEGER_BASE_ARENA_C

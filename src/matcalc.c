#include "krueger_base.h"
#include "krueger_base.c"

global s32 option;

typedef struct {
  u32 rows;
  u32 cols;
  s32 *ptr;
} Matrix;

global Matrix *matrices;
// global Matrix *history;

#define mat_get(mat, i, j) ((mat).ptr[(i)*(mat).cols + (j)])
#define mat_put(mat, i, j, x) ((mat)->ptr[(i)*(mat)->cols + (j)] = (x))

internal u32
pick_mat_idx() {
  for (uxx i = 0; i < sb_len(matrices); ++i) {
    printf("[%zu] ", i + 1);
    if ((i+1) == sb_len(matrices)) printf("\n");
  }
  u32 idx;
  scanf("%d", &idx);
  return(idx - 1);
}

internal void
show_mat(Matrix mat) {
  for (u32 i = 0; i < mat.rows; ++i) {
    printf("[ ");
    for (u32 j = 0; j < mat.cols; ++j) {
      s32 val = mat.ptr[i*mat.cols + j];
      printf("%d ", val);
    }
    printf("]\n");
  }
}

internal void
show_mat_item_pick(Matrix mat, u32 row, u32 col) {
  for (u32 i = 0; i < mat.rows; ++i) {
    printf("[ ");
    for (u32 j = 0; j < mat.cols; ++j) {
      if ((i == row) && (j == col)) {
        printf("* ");
      } else {
        s32 val = mat.ptr[i*mat.cols + j];
        printf("%d ", val);
      }
    }
    printf("]\n");
  }
}

// IMPORTANT: you need to check if a's rows and cols matches with the b's rows and cols.
internal Matrix
mat_add(Matrix a, Matrix b) {
  Matrix result = {
    .rows = a.rows,
    .cols = a.cols,
    .ptr = malloc(a.rows*a.cols*sizeof(s32)),
  };
  for (u32 i = 0; i < result.rows; ++i) {
    for (u32 j = 0; j < result.cols; ++j) {
      u32 val = mat_get(a, i, j) + mat_get(b, i, j);
      mat_put(&result, i, j, val);
    }
  }
  return(result);
}

int
main(void) {
  // TODO: check input
  for (b32 quit = false; !quit;) {
    printf("[matcalc]:\n");
    printf("[1] add\n");
    printf("[2] fill\n");
    printf("[3] show\n");
    printf("[4] add\n");
    printf("[0] quit\n");
    scanf("%d", &option);
    switch (option) {
      case 1: {
        Matrix mat = {0};
        printf("enter row count:\n");
        scanf("%d", &mat.rows);
        printf("enter col count:\n");
        scanf("%d", &mat.cols);
        mat.ptr = malloc(mat.rows*mat.cols*sizeof(s32));
        memset(mat.ptr, 0, mat.rows*mat.cols);
        sb_push(matrices, mat);
        log_info("added matrix: [%zu]", sb_len(matrices));
      } break;
      case 2: {
        if (sb_len(matrices) > 0) {
          printf("pick matrix:\n");
          u32 idx = pick_mat_idx();
          Matrix *mat = matrices + idx;
          for (u32 i = 0; i < mat->rows; ++i) {
            for (u32 j = 0; j < mat->cols; ++j) {
              show_mat_item_pick(*mat, i, j);
              printf("enter value:\n");
              s32 val;
              scanf("%d", &val);
              mat_put(mat, i, j, val);
            }
          }
          log_info("filled matrix: [%d]", idx);
          show_mat(*mat);
        } else {
          log_info("there is no matrix to fill");
        }
      } break;
      case 3: {
        if (sb_len(matrices) > 0) {
          printf("pick matrix:\n");
          u32 idx = pick_mat_idx();
          Matrix mat = matrices[idx];
          log_info("showed matrix: [%d]", idx);
          show_mat(mat);
        } else {
          log_info("there is no matrix to show");
        }
      } break;
      case 4: {
        if (sb_len(matrices) > 0) {
          printf("pick matrix: [*] + [?]\n");
          u32 idx0 = pick_mat_idx();
          Matrix a = matrices[idx0];

          printf("pick matrix: [%d] + [*]\n", idx0+1);
          u32 idx1 = pick_mat_idx();
          Matrix b = matrices[idx1];

          log_info("added matrices: [%d] + [%d]", idx0+1, idx1+1);
          Matrix result = mat_add(a, b);
          show_mat(result);
        } else {
          log_info("there is no matrix to add");
        }
      } break;
      case 0: {
        quit = true;
      } break;
      default: {
        log_error("invalid option");
      }
    }
  }
  return(0);
}

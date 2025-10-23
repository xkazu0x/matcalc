#include "krueger_base.h"
#include "krueger_base.c"

typedef u32 Option_Type;

global Option_Type root_option;
global Option_Type stack_option;
global Option_Type operation_option;

enum {
  ROOT_OPTION_QUIT,
  ROOT_OPTION_STACK,
  ROOT_OPTION_OPERATION,
  ROOT_OPTION_MAX,
};

enum {
  STACK_OPTION_BACK,
  STACK_OPTION_MAKE,
  STACK_OPTION_FILL,
  STACK_OPTION_SHOW,
  STACK_OPTION_CLEAR,
  STACK_OPTION_MAX,
};

enum {
  OPERATION_OPTION_BACK,
  OPERATION_OPTION_ADD,
  OPERATION_OPTION_SUB,
  OPERATION_OPTION_MUL,
  OPERATION_OPTION_MAX,
};

typedef struct {
  u32 rows;
  u32 cols;
  s32 *ptr;
} Matrix;

global Matrix *matrices;

#define mat_get(mat, i, j) ((mat).ptr[(i)*(mat).cols + (j)])
#define mat_put(mat, i, j, x) ((mat)->ptr[(i)*(mat)->cols + (j)] = (x))

internal u32
pick_mat_idx(void) {
  printf("pick matrix:\n");
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

internal Matrix
mat_sub(Matrix a, Matrix b) {
  Matrix result = {
    .rows = a.rows,
    .cols = b.cols,
    .ptr = malloc(a.rows*a.cols*sizeof(s32)),
  };
  for (u32 i = 0; i < result.rows; ++i) {
    for (u32 j = 0; j < result.cols; ++j) {
      u32 val = mat_get(a, i, j) - mat_get(b, i, j);
      mat_put(&result, i, j, val);
    }
  }
  return(result);
}

internal Matrix
mat_mul(Matrix a, Matrix b) {
  Matrix result = {
    .rows = a.rows,
    .cols = b.cols,
    .ptr = malloc(a.rows*a.cols*sizeof(s32)),
  };
  s32 *a_tmp = 0;
  s32 *b_tmp = 0;
  for (u32 i = 0; i < result.rows; ++i) {
    for (u32 j = 0; j < result.cols; ++j) {
      sb_clear(a_tmp);
      sb_clear(b_tmp);
      for (u32 col = 0; col < a.cols; ++col) {
        sb_push(a_tmp, mat_get(a, i, col));
      }
      for (u32 row = 0; row < b.rows; ++row) {
        sb_push(b_tmp, mat_get(b, row, j));
      }
      u32 val = 0;
      for (u32 k = 0; k < a.cols; ++k) {
        val += a_tmp[k]*b_tmp[k];
      }
      mat_put(&result, i, j, val);
    }
  }
  sb_free(a_tmp);
  sb_free(b_tmp);
  return(result);
}

int
main(void) {
  // TODO: check input
  for (b32 quit = false; !quit;) {
    printf("[matcalc]\n");
    printf("[%d] stack\n", ROOT_OPTION_STACK);
    printf("[%d] operation\n", ROOT_OPTION_OPERATION);
    printf("[%d] quit\n", ROOT_OPTION_QUIT);
    scanf("%d", &root_option);

    switch (root_option) {
      case ROOT_OPTION_QUIT: {
        quit = true;
      } break;

      case ROOT_OPTION_STACK: {
        printf("[stack option]\n");
        printf("[%d] make\n", STACK_OPTION_MAKE);
        printf("[%d] fill\n", STACK_OPTION_FILL);
        printf("[%d] show\n", STACK_OPTION_SHOW);
        printf("[%d] clear\n", STACK_OPTION_CLEAR);
        printf("[%d] back\n", STACK_OPTION_BACK);
        scanf("%d", &stack_option);

        switch (stack_option) {
          case STACK_OPTION_BACK: {
            // NOTE: do nothing and it will go back to the root menu
          } break;

          case STACK_OPTION_MAKE: {
            u32 rows = 0;
            u32 cols = 0;
            printf("enter row count:\n");
            scanf("%d", &rows);
            printf("enter col count:\n");
            scanf("%d", &cols);
            uxx buf_size = rows*cols*sizeof(s32);
            Matrix mat = {
              .rows = rows,
              .cols = cols,
              .ptr = malloc(buf_size),
            };
            mem_zero(mat.ptr, buf_size);
            sb_push(matrices, mat);
            log_info("make matrix: [%zu]", sb_len(matrices));
          } break;

          case STACK_OPTION_FILL: {
            if (sb_len(matrices) > 0) {
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
              log_info("fill matrix: [%d]", idx+1);
              show_mat(*mat);
            } else {
              log_warn("there is no matrix to fill");
            }
          } break;

          case STACK_OPTION_SHOW: {
            if (sb_len(matrices) > 0) {
#if 0
              u32 idx = pick_mat_idx();
              Matrix mat = matrices[idx];
              log_info("show matrix: [%d]", idx+1);
              show_mat(mat);
#else
              for (uxx i = 0; i < sb_len(matrices); ++i) {
                Matrix mat = matrices[i];
                log_info("show matrix: [%d]", i+1);
                show_mat(mat);
              }
#endif
            } else {
              log_warn("there is no matrix to show");
            }
          } break;

          case STACK_OPTION_CLEAR: {
            sb_clear(matrices);
          } break;

          default: {
            log_warn("invalid option");
          }
        } // switch (stack_option)
      } break;

      case ROOT_OPTION_OPERATION: {
        printf("[operation option]\n");
        printf("[%d] add\n", OPERATION_OPTION_ADD);
        printf("[%d] sub\n", OPERATION_OPTION_SUB);
        printf("[%d] mul\n", OPERATION_OPTION_MUL);
        printf("[%d] back\n", OPERATION_OPTION_BACK);
        scanf("%d", &operation_option);

        switch (operation_option) {
          case OPERATION_OPTION_ADD: {
            if (sb_len(matrices) > 0) {
              printf("[*] + [_]\n");
              u32 idx0 = pick_mat_idx();
              Matrix a = matrices[idx0];

              printf("[%d] + [*]\n", idx0+1);
              u32 idx1 = pick_mat_idx();
              Matrix b = matrices[idx1];

              if ((a.rows == b.rows) && (a.cols == b.cols)) {
                log_info("add matrix: [%d] + [%d]", idx0+1, idx1+1);
                Matrix result = mat_add(a, b);
                show_mat(result);
              } else {
                log_warn("matrix [%d]'s rows, cols must match [%d]'s rows, cols", idx0+1, idx1+1);
              }
            } else {
              log_warn("there is no matrix to add");
            }
          } break;

          case OPERATION_OPTION_SUB: {
            if (sb_len(matrices) > 0) {
              printf("[*] - [_]\n");
              u32 idx0 = pick_mat_idx();
              Matrix a = matrices[idx0];

              printf("[%d] - [*]\n", idx0+1);
              u32 idx1 = pick_mat_idx();
              Matrix b = matrices[idx1];

              if ((a.rows == b.rows) && (a.cols == b.cols)) {
                log_info("sub matrix: [%d] - [%d]", idx0+1, idx1+1);
                Matrix result = mat_sub(a, b);
                show_mat(result);
              } else {
                log_warn("matrix [%d]'s rows, cols must match [%d]'s rows, cols", idx0+1, idx1+1);
              }
            } else {
              log_warn("there is no matrix to sub");
            }
          } break;

          case OPERATION_OPTION_MUL: {
            if (sb_len(matrices) > 0) {
              printf("[*] . [_]\n");
              u32 idx0 = pick_mat_idx();
              Matrix a = matrices[idx0];

              printf("[%d] . [*]\n", idx0+1);
              u32 idx1 = pick_mat_idx();
              Matrix b = matrices[idx1];

              if ((a.rows == b.cols) && (a.cols == b.rows)) {
                log_info("mul matrix: [%d] . [%d]", idx0+1, idx1+1);
                Matrix result = mat_mul(a, b);
                show_mat(result);
              } else {
                log_warn("matrix [%d]'s rows, cols must match [%d]'s cols, rows", idx0+1, idx1+1);
              }
            } else {
              log_warn("there is no matrix to mul");
            }
          } break;
        } // switch (operation_option)
      }

      default: {
        log_warn("invalid option");
      }
    } // switch (root_option)
  }
  return(0);
}

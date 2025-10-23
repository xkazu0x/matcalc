#include "krueger_base.h"
#include "krueger_base.c"

typedef u32 Option_Type;

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
  OPERATION_OPTION_TPS,
  OPERATION_OPTION_DET,
  OPERATION_OPTION_MAX,
};

typedef struct {
  u32 rows;
  u32 cols;
  s32 *ptr;
} Matrix;

global Option_Type root_option;
global Option_Type stack_option;
global Option_Type operation_option;

global Matrix *matrices;

#define mat_get(mat, i, j) ((mat).ptr[(i)*(mat).cols + (j)])
#define mat_put(mat, i, j, x) ((mat)->ptr[(i)*(mat)->cols + (j)] = (x))

#define mat_get_idx(mat, idx, x) ((mat).ptr[(idx)])
#define mat_put_idx(mat, idx, x) ((mat).ptr[(idx)] = (x))

internal Matrix
mat_alloc(u32 rows, u32 cols) {
  uxx size = rows*cols*sizeof(s32);
  Matrix result = {
    .rows = rows,
    .cols = cols,
    .ptr = malloc(size),
  };
  mem_zero(result.ptr, size);
  return(result);
}

internal void
mat_release(Matrix mat) {
  if (mat.ptr) {
    free(mat.ptr);
  }
  mat.cols = 0;
  mat.rows = 0;
  mat.ptr = 0;
}

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
      s32 val = mat_get(a, i, j) + mat_get(b, i, j);
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
      s32 val = mat_get(a, i, j) - mat_get(b, i, j);
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
      s32 val = 0;
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

internal s32
mat_det(Matrix mat) {
  s32 result = 0;
  if (mat.rows == 1 && mat.cols == 1) {
    result = mat_get(mat, 0, 0);
  } else if (mat.rows == 2 && mat.cols == 2) {
    s32 a = mat_get(mat, 0, 0);
    s32 b = mat_get(mat, 0, 1);
    s32 c = mat_get(mat, 1, 0);
    s32 d = mat_get(mat, 1, 1);
    result = (a*d) - (b*c);
  } else {
    s32 sign = 1;
    u32 rows = mat.rows - 1;
    u32 cols = mat.cols - 1;
    u32 dim = mat.cols;
    for (u32 i = 0; i < dim; ++i) {
      Matrix tmp = mat_alloc(rows, cols);
      u32 idx = 0;
      for (u32 row = 0; row < mat.rows; ++row) {
        for (u32 col = 0; col < mat.cols; ++col) {
          s32 val = mat_get(mat, row, col);
          if ((row == 0) || (col == i)) {
          } else {
            mat_put_idx(tmp, idx, val);
            ++idx;
          }
        }
      }
      result += sign*mat_get(mat, 0, i)*mat_det(tmp);
      sign *= -1;
      mat_release(tmp);
    }
  }
  return(result);
}

internal Matrix
mat_tsp(Matrix mat) {
  Matrix result = {
    .rows = mat.cols,
    .cols = mat.rows,
    .ptr = malloc(mat.rows*mat.cols*sizeof(u32)),
  };
  for (u32 i = 0; i < result.rows; ++i) {
    for (u32 j = 0; j < result.cols; ++j) {
      s32 val = mat_get(mat, j, i);
      mat_put(&result, i, j, val);
    }
  }
  return(result);
}

int
main(void) {
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
            Matrix mat = mat_alloc(rows, cols);
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
              for (uxx i = 0; i < sb_len(matrices); ++i) {
                Matrix mat = matrices[i];
                log_info("show matrix: [%d]", i+1);
                show_mat(mat);
              }
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
      } break; // case ROOT_OPTION_STACK

      case ROOT_OPTION_OPERATION: {
        printf("[operation option]\n");
        printf("[%d] add\n", OPERATION_OPTION_ADD);
        printf("[%d] sub\n", OPERATION_OPTION_SUB);
        printf("[%d] mul\n", OPERATION_OPTION_MUL);
        printf("[%d] tps\n", OPERATION_OPTION_TPS);
        printf("[%d] det\n", OPERATION_OPTION_DET);
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

          case OPERATION_OPTION_DET: {
            if (sb_len(matrices) > 0) {
              u32 idx = pick_mat_idx();
              Matrix mat = matrices[idx];
              if (mat.rows == mat.cols) {
                s32 result = mat_det(mat);
                log_info("det matrix: [%d]", idx+1);
                printf("%d\n", result);
              } else {
                log_warn("matrix[%d] row and col count must be equal", idx+1);
              }
            } else {
              log_warn("there is no matrix to transpose");
            }
          } break;

          case OPERATION_OPTION_TPS: {
            if (sb_len(matrices) > 0) {
              u32 idx = pick_mat_idx();
              Matrix mat = matrices[idx];
              Matrix result = mat_tsp(mat);
              sb_push(matrices, result);
              log_info("tps matrix: [%d]", idx+1);
              show_mat(result);
            } else {
              log_warn("there is no matrix to transpose");
            }
          } break;

          default: {
            log_warn("invalid option");
          }
        } // switch (operation_option)
      } break; // case ROOT_OPTION_OPERATION

      default: {
        log_warn("invalid option");
      }
    } // switch (root_option)
  }
  return(0);
}

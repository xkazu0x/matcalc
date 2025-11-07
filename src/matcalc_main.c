#include "krueger_base.h"
#include "krueger_base.c"

typedef struct Matrix Matrix;
struct Matrix {
  Matrix *next;
  Matrix *prev;
  u32 id;
  u32 row_count;
  u32 col_count;
  s32 *data;
};

typedef struct Matrix_List Matrix_List;
struct Matrix_List {
  Arena arena;
  u32 count;
  Matrix *first;
  Matrix *last;
  Matrix *first_free;
};

internal Matrix *
matrix_alloc(Matrix_List *list) {
  Matrix *result = list->first_free;
  if (!result) {
    result = push_struct(&list->arena, Matrix);
  } else {
    sll_stack_pop(list->first_free);
  }
  mem_zero_struct(result);
  dll_push_back(list->first, list->last, result);
  return(result);
}

internal void
matrix_release(Matrix_List *list, Matrix *matrix) {
  dll_remove(list->first, list->last, matrix);
  sll_stack_push(list->first_free, matrix);
}

internal Matrix_List *
matrix_list_alloc(void) {
  Arena arena = arena_alloc(MB(64));
  Matrix_List *result = push_struct(&arena, Matrix_List);
  result->arena = arena;
  return(result);
}

internal Matrix *
matrix_list_push(Matrix_List *list, u32 row_count, u32 col_count) {
  Matrix *matrix = matrix_alloc(list);
  matrix->id = 1 + list->count++;
  matrix->row_count = row_count;
  matrix->col_count = col_count;
  uxx data_size = row_count*col_count*sizeof(s32);
  matrix->data = malloc(data_size);
  mem_zero(matrix->data, data_size);
  return(matrix);
}

internal void
matrix_list_remove(Matrix_List *list, Matrix *matrix) {
  b32 found = false;
  for (Matrix *mat = list->first; mat != 0; mat = mat->next) {
    if (found) mat->id -= 1;
    if (mat == matrix) {
      found = true;
    }
  }
  free(matrix->data);
  matrix_release(list, matrix);
  list->count -= 1;
}

internal void
matrix_list_print(Matrix_List *list) {
  printf("[matrix list]:\n");
  for (Matrix *matrix = list->first; matrix != 0; matrix = matrix->next) {
    printf("[%d] ", matrix->id);
  }
  printf("\n");
}

internal b32
matrix_list_contains(Matrix_List *list, u32 id) {
  b32 result = false;
  for (Matrix *matrix = list->first; matrix != 0; matrix = matrix->next) {
    if (matrix->id == id) result = true;
  }
  return(result);
}

internal s32
matrix_det(Matrix_List *list, Matrix *matrix) {
  s32 result = 0;
  if (matrix->row_count == 1 && matrix->col_count == 1) {
    result = matrix->data[0];
  } else if (matrix->row_count == 2 && matrix->col_count == 2) {
    s32 a = matrix->data[0];
    s32 b = matrix->data[1];
    s32 c = matrix->data[2];
    s32 d = matrix->data[3];
    result = a*d - b*c;
  } else {
    s32 sign = 1;
    u32 col_count = matrix->col_count - 1;
    u32 row_count = matrix->row_count - 1;
    for (u32 i = 0; i < matrix->col_count; ++i) {
      Matrix *temp = matrix_list_push(list, col_count, row_count);
      u32 index = 0;
      for (u32 row = 0; row < matrix->row_count; ++row) {
        for (u32 col = 0; col < matrix->col_count; ++col) {
          if ((row != 0) && (col != i)) {
            s32 value = matrix->data[row*matrix->col_count + col];
            temp->data[index++] = value;
          }
        }
      }
      result += sign*matrix->data[i]*matrix_det(list, temp);
      sign *= -1;
      matrix_list_remove(list, temp);
    }
  }
  return(result);
}

internal void
matrix_print_info(Matrix *matrix) {
  printf("[matrix]: [%d]\n[row: %d] [col: %d]\n",
         matrix->id, matrix->row_count, matrix->col_count);
}

internal void
matrix_print_data(Matrix *matrix) {
  for (u32 i = 0; i < matrix->row_count; ++i) {
    printf("[ ");
    for (u32 j = 0; j < matrix->col_count; ++j) {
      printf("%d ", matrix->data[i*matrix->col_count + j]);
    }
    printf("]\n");
  }
}

internal void
matrix_print_data_pick(Matrix *matrix, u32 row, u32 col) {
  for (u32 i = 0; i < matrix->row_count; ++i) {
    printf("[ ");
    for (u32 j = 0; j < matrix->col_count; ++j) {
      if ((i == row) && (j == col)) {
        printf("* ");
      } else {
        printf("%d ", matrix->data[i*matrix->col_count + j]);
      }
    }
    printf("]\n");
  }
}

internal u32
u32_from_input(void) {
  u32 result = 0;
  u32 scan_result = scanf("%d", &result);
  while (scan_result == 0) {
    printf("Invalid input. Please enter a valid option.\n");
    scan_result = scanf("%d", &result);
    char ch = (char)getchar();
    if (ch == '\n') continue;
  }
  return(result);
}

internal Matrix *
matrix_from_id(Matrix_List *list, u32 id) {
  Matrix *result = 0;
  for (Matrix *matrix = list->first; matrix != 0; matrix = matrix->next) {
    if (matrix->id == id) {
      result = matrix;
      break;
    }
  }
  return(result);
}

internal Matrix *
matrix_from_input(Matrix_List *list) {
  matrix_list_print(list);
  printf("[input]: enter matrix id:\n");
  u32 id = u32_from_input();
  while (!matrix_list_contains(list, id)) {
    matrix_list_print(list);
    printf("[warn]: please enter a valid matrix id\n");
    id = u32_from_input();
  }
  Matrix *result = matrix_from_id(list, id);
  return(result);
}

typedef u32 Menu_Option;
enum {
  MENU_QUIT,
  MENU_EDIT_LIST,
  MENU_OPERATION,
};

typedef u32 Edit_List_Option;
enum {
  EDIT_LIST_BACK,
  EDIT_LIST_MAKE,
  EDIT_LIST_FILL,
  EDIT_LIST_PRINT,
  EDIT_LIST_DELETE,
  EDIT_LIST_MAX,
};

typedef u32 Operation_Option;
enum {
  OPERATION_BACK,
  OPERATION_ADD,
  OPERATION_SUB,
  OPERATION_MUL,
  OPERATION_DET,
  OPERATION_T,
  OPERATION_MAX,
};

internal void
edit_list(Matrix_List *list) {
  b32 back = false;
  Edit_List_Option edit_list_opt = 0;
  do {
    printf("[matcalc]:\n");
    printf("[%d] make\n", EDIT_LIST_MAKE);
    printf("[%d] fill\n", EDIT_LIST_FILL);
    printf("[%d] print\n", EDIT_LIST_PRINT);
    printf("[%d] delete\n", EDIT_LIST_DELETE);
    printf("[%d] back\n", EDIT_LIST_BACK);
    printf("[input]: choose an option\n");
    edit_list_opt = u32_from_input();
    switch (edit_list_opt) {
      case EDIT_LIST_BACK: {
        back = true;
      } break;
      case EDIT_LIST_MAKE: {
        printf("[input]: enter row count:\n");
        u32 row = u32_from_input();
        printf("[input]: enter col count:\n");
        u32 col = u32_from_input();
        Matrix *matrix = matrix_list_push(list, row, col);
        matrix_print_info(matrix);
      } break;
      case EDIT_LIST_FILL: {
        if (list->count) {
          Matrix *matrix = matrix_from_input(list);
          for (u32 row = 0; row < matrix->row_count; ++row) {
            for (u32 col = 0; col < matrix->col_count; ++col) {
              matrix_print_info(matrix);
              matrix_print_data_pick(matrix, row, col);
              printf("[input]: enter value:\n");
              u32 value = u32_from_input();
              matrix->data[row*matrix->col_count + col] = value;
            }
          }
          matrix_print_info(matrix);
          matrix_print_data(matrix);
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      case EDIT_LIST_PRINT: {
        if (list->count) {
          for (Matrix *matrix = list->first; matrix != 0; matrix = matrix->next) {
            matrix_print_info(matrix);
            matrix_print_data(matrix);
          }
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      case EDIT_LIST_DELETE: {
        if (list->count) {
          Matrix *matrix = matrix_from_input(list);
          matrix_list_remove(list, matrix);
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      default: {
        printf("[warn]: choose a valid option\n");
      };
    }
  } while (!back); 
}

internal void
operation(Matrix_List *list) {
  b32 back = false;
  Operation_Option op_opt = 0;
  do {
    printf("[matcalc]:\n");
    printf("[%d] add\n", OPERATION_ADD);
    printf("[%d] sub\n", OPERATION_SUB);
    printf("[%d] mul\n", OPERATION_MUL);
    printf("[%d] det\n", OPERATION_DET);
    printf("[%d] t\n", OPERATION_T);
    printf("[%d] back\n", OPERATION_BACK);
    printf("[input]: choose an option\n");
    op_opt = u32_from_input();
    switch (op_opt) {
      case OPERATION_BACK: {
        back = true;
      } break;
      case OPERATION_ADD: {
        if (list->count) {
          printf("[add operation]: [*] + [_]\n");
          Matrix *left = matrix_from_input(list);
          printf("[add operation]: [%d] + [*]\n", left->id);
          Matrix *right = matrix_from_input(list);
          if ((left->row_count == right->row_count) &&
            (left->col_count == right->col_count)) {
            Matrix *result = matrix_list_push(list, left->row_count, right->col_count);
            for (u32 i = 0; i < result->row_count; ++i) {
              for (u32 j = 0; j < result->col_count; ++j) {
                u32 index = i*result->col_count + j;
                s32 left_value = left->data[index];
                s32 right_value = right->data[index];
                result->data[index] = left_value + right_value;
              }
            }
            matrix_print_info(result);
            matrix_print_data(result);
          } else {
            printf("[warn]: matrices must have the same dimension\n");
          }
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      case OPERATION_SUB: {
        if (list->count) {
          printf("[sub operation]: [*] - [_]\n");
          Matrix *left = matrix_from_input(list);
          printf("[sub operation]: [%d] - [*]\n", left->id);
          Matrix *right = matrix_from_input(list);
          if ((left->row_count == right->row_count) &&
            (left->col_count == right->col_count)) {
            Matrix *result = matrix_list_push(list, left->row_count, right->col_count);
            for (u32 i = 0; i < result->row_count; ++i) {
              for (u32 j = 0; j < result->col_count; ++j) {
                u32 index = i*result->col_count + j;
                s32 left_value = left->data[index];
                s32 right_value = right->data[index];
                result->data[index] = left_value - right_value;
              }
            }
            matrix_print_info(result);
            matrix_print_data(result);
          } else {
            printf("[warn]: matrices must have the same dimension\n");
          }
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      case OPERATION_MUL: {
        if (list->count) {
          printf("[mul operation]: [*] . [_]\n");
          Matrix *left = matrix_from_input(list);
          printf("[mul operation]: [%d] . [*]\n", left->id);
          Matrix *right = matrix_from_input(list);
          if ((left->row_count == right->col_count) &&
            (left->col_count == right->row_count)) {
            Matrix *result = matrix_list_push(list, left->row_count, right->col_count);
            for (u32 i = 0; i < result->row_count; ++i) {
              for (u32 j = 0; j < result->col_count; ++j) {
                s32 value = 0;
                for (u32 k = 0; k < left->col_count; ++k) {
                  s32 left_value = left->data[i*left->col_count + k];
                  s32 right_value = right->data[k*right->col_count + j];
                  value += left_value*right_value;
                }
                result->data[i*result->col_count + j] = value;
              }
            }
            matrix_print_info(result);
            matrix_print_data(result);
          } else {
            printf("[warn]: left's row, col count must match right's col, row count\n");
          }
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      case OPERATION_DET: {
        if (list->count) {
          printf("[det operation]: |*|\n");
          Matrix *matrix = matrix_from_input(list);
          if (matrix->row_count == matrix->col_count) {
            s32 result = matrix_det(list, matrix);
            matrix_print_info(matrix);
            matrix_print_data(matrix);
            printf("det: %d\n", result);
          } else {
            printf("[warn]: it is only possible to calculate the determinant of a square matrix\n");
          }
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      case OPERATION_T: {
        if (list->count) {
          printf("[t operation]:\n");
          Matrix *matrix = matrix_from_input(list);
          Matrix *result = matrix_list_push(list, matrix->col_count, matrix->row_count);
          for (u32 i = 0; i < result->row_count; ++i) {
            for (u32 j = 0; j < result->col_count; ++j) {
              s32 value = matrix->data[j*matrix->col_count + i];
              result->data[i*result->col_count + j] = value;
            }
          }
          matrix_print_info(result);
          matrix_print_data(result);
        } else {
          printf("[info]: the list is empty\n");
        }
      } break;
      default: {
        printf("[warn]: choose a valid option\n");
      };
    }
  } while (!back); 
}

int
main(void) {
  Matrix_List *list = matrix_list_alloc();

  b32 quit = false;
  Menu_Option menu_opt = 0;

  do {
    printf("[matcalc]:\n");
    printf("[%d] edit list\n", MENU_EDIT_LIST);
    printf("[%d] operation\n", MENU_OPERATION);
    printf("[%d] quit\n", MENU_QUIT);
    printf("[input]: choose an option\n");
    menu_opt = u32_from_input();
    switch (menu_opt) {
      case MENU_QUIT:       quit = true; break;
      case MENU_EDIT_LIST:  edit_list(list); break;
      case MENU_OPERATION:  operation(list); break;
      default:              printf("[warn]: choose a valid option\n");
    }
  } while(!quit);

  return(0);
}

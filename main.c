#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <ncurses.h>
#include <math.h>

#define MAX_BUF 100
#define THR 1e-6
#define MAX_STR 1000


static const wchar_t* MESSAGE_1 = L"Число показателей m ";
static const wchar_t* MESSAGE_2 = L"Число лет n ";
static const wchar_t* MESSAGE_3 = L"alpha ";

int M = 10000;
int N = 10000;
int alpha = -1;

void askparams(int *pm, int m_min, int m_max, int *pn, int n_min, int n_max){

  do{
    addwstr(MESSAGE_1);
    printw("[%d-%d]: ", m_min, m_max);
    scanw("%d", pm);
    if (*pm < m_min || *pm > m_max){
      addwstr(L"Значение m введено неверно\n");
    }
  }while(*pm < m_min || *pm > m_max);

  do{
    addwstr(MESSAGE_2);
    printw("[%d-%d]: ", n_min, n_max);
    scanw("%d", pn);
    if (*pn < n_min || *pn > n_max){
      addwstr(L"Значение n введено неверно\n");
    }
  }while(*pn < n_min || *pn > n_max);

  do{
    addwstr(MESSAGE_3);
    printw("[0-1]: ");
    scanw("%f", &alpha);
    if (alpha < 0 || alpha > 1){
      addwstr(L"Значение alpha введено неверно\n");
    }
  }while(alpha < 0 || alpha > 1);
  erase();
}

wchar_t** load_pokazat(char* filename){
   
   wchar_t** buf;
   wchar_t** res;
   wchar_t*  str;
   int max_size, len, i;
   int loaded;
   wchar_t buf_str[MAX_STR];
   FILE *f;


   max_size = 10;
   loaded = -1;

   buf = malloc(max_size * sizeof(wchar_t*));
   f= fopen(filename, "r");
   if (!f)
     return 0;
   do{
      str = fgetws(buf_str, MAX_STR, f);
      if (str){
        loaded++;
        len = wcsnlen(str, MAX_STR);
        if (loaded >= max_size){
          max_size = max_size + 10;
          buf = realloc(buf, max_size * sizeof(wchar_t*));
        }
        buf[loaded] = malloc((len+1) * sizeof(wchar_t));
        wcsncpy(buf[loaded], str, len);
      }
   }while(str);

   fclose(f);

   res = malloc((loaded+2) * sizeof(wchar_t*));
   for(i=0; i<loaded+1; i++){
     res[i] = buf[i];
   }
   res[loaded+1] = 0;
   free(buf);
   return res;
}

double* load_line(FILE* f){
  double* buf;
  double cur;
  int loaded, max_size, pos;
  char tmp[MAX_STR];
  char c;
  max_size = 1;
  loaded = 0;
  pos = 0;

  buf = malloc(max_size * sizeof(double));

  do{
    c = fgetc(f);
    if (pos >= MAX_STR){
      break;
    }
    if (c !=' ' && c != '\n' && c != EOF){
      tmp[pos++] = c;
    }else{
      tmp[pos++] = 0;
      if (c == ' ' || c == '\n'){
        if (pos > 1){
        cur = atof(tmp);
        }else{
        cur = NAN;
        }
        pos = 0;
//        printf("%lf\n", cur);
        if(loaded >= max_size){
          max_size = max_size + 1;
          buf = realloc(buf, max_size * sizeof(double));
        }
        buf[loaded] = cur;
        loaded++;
      }
      if (c != ' ')
        break;
    }
  }while(c!= '\n' && c != EOF);
//  printf("end %d\n", loaded);
  buf = realloc(buf, (loaded+1)*sizeof(double));
  buf[loaded] = INFINITY;
  return buf;
}
double** load_data(char* filename){
   
   double** buf;
   double* row;
   int max_size, len, i;
   int loaded;
   FILE *f;


   max_size = 1;
   loaded = 0;

   buf = malloc(max_size * sizeof(double*));
   f= fopen(filename, "r");
   if (!f)
     return 0;
   do{
      row = load_line(f);
      len = 0;
      for(i=0; !isinf(row[i]); i++){
        len = i+1;
      }
      if (len > 0){
        if (loaded >= max_size){
          max_size = max_size + 1;
          buf = realloc(buf, max_size * sizeof(double*));
        }
        buf[loaded] = row;
        loaded++;
//        printf("lines: %d\n", loaded);
      }
   }while(len > 0);

   fclose(f);

   buf = realloc(buf, (loaded+1) * sizeof(double*));
   buf[loaded] = 0;
   return buf;
}
int display(const wchar_t** pokaz, int i, int j, double c){
  int a = -1;
  const wchar_t *str1 = pokaz[i];
  const wchar_t *str2 = pokaz[j];

//  mvprintw(2, 3, "%d => 1, %d => 2", i+1, j+1);
  mvprintw(3, 3, "1 - [%d] ", i+1);
//  mvaddwstr(3, 3, L"1 - ");
  mvaddwstr(3, 14, str1);
  mvprintw(5, 3, "2 - [%d] ", j+1);
//  mvaddwstr(5, 3, L"2 - ");
  mvaddwstr(5, 14, str2);
  mvaddwstr(7, 3, L"Коэффициент корреляции:        ");
  mvprintw(7, 27, "%f", c);
  mvaddwstr(9, 3, L"Выберите один из вариантов:");
  mvaddwstr(11, 3, L"1. Показатель 1->2 (1 влияет на 2, то есть 2 зависит от 1)");
  mvaddwstr(12, 3, L"2. Показатель 2->1 (2 влияет на 1, то есть 1 зависит от 2)");
  mvaddwstr(13, 3, L"3. Показатель 1<->2 (зависят друг от друга)");
  mvaddwstr(14, 3, L"0. Выход из программы");
  while (a < 0 || a > 3){
    mvaddwstr(16, 3, L"Введите число 1,2,3 или 0:       ");
    refresh();
    mvscanw(16,30, "%d", &a);
  }
  return a;
}
double calc_corr(const double* x, const double* y, int len){
  double mx = 0;
  double my = 0;
  double cxcy = 0;
  double cxx = 0;
  double cyy = 0;
  int i;

  if (len < 1)
    return NAN;

  for(i=0; i<len; i++){
    if (isnormal(x[i]) && isnormal(y[i])){
      mx += x[i];
      my += y[i];
    }
  }
  mx /= len;
  my /= len;

  for(i=0; i<len; i++){
    if (isnormal(x[i]) && isnormal(y[i])){
      cxcy += (x[i] - mx)*(y[i]-my);
      cxx += (x[i]-mx)*(x[i]-mx);
      cyy += (y[i]-my)*(y[i]-my);
    }
  }

  if (cxx < THR && cyy < THR){
    return 1;
  }else if (cxx < THR || cyy < THR){
    return 0;
  }
  return cxcy / sqrt(cxx * cyy);

}
double** create_matrix(int len){
  int i,j;
  double ** matrix;

  matrix = malloc((len+1)*sizeof(double*));

  for(i=0; i<len; i++){
    matrix[i] = malloc((len+1)*sizeof(double));
  }
  matrix[len] = 0;

  for(i=0; i<len; i++){
    matrix[i][i] = 1.0;
    for(j=i+1; j<len; j++){
      matrix[i][j] = NAN;
      matrix[j][i] = NAN;
    }
    matrix[i][len] = INFINITY;
  }
  return matrix;
}
void destroy_matrix(double** matrix, int len){
  int i;

  for(i=0; i<len; i++){
    free(matrix[i]);
  }
  free(matrix);
}
void destroy_array(void** buf){
  int i;

  for(i=0; buf[i]; i++){
    free(buf[i]);
  }
  free(buf);
}
void save_correlation(char* filename, double** matrix, int len){
  int i,j;
  FILE * f;

  f = fopen(filename, "w");
  for(i=0; i<len; i++){
    for(j=0; j<len; j++){
      fprintf(f, "%f", fabs(matrix[i][j]));
      if (j < len-1){
        fprintf(f, " ");
      }else{
        fprintf(f, "\n");
      }
    }
  }
  fclose(f);
}
void save_adjustment(char* filename, double** matrix, int len){
  int i,j;
  FILE *f;
  double c;

  f = fopen(filename, "w");
  for(i=0; i<len; i++){
    for(j=0; j<len; j++){
      c = matrix[i][j];
      if (fabs(c) > THR)
        fprintf(f, "1 ");
      else if (isnan(c))
        fprintf(f, ". ");
      else
        fprintf(f, "0 ");
    }
    fprintf(f, "\n");
  }
  fclose(f);
}
int update_arc(double** matrix, const wchar_t** pokaz, int i, int j, double c){
  int a;

  do{
    a = display(pokaz, i, j, c);
  }while (a < 0 || a > 3);

  if (a == 1){
    matrix[i][j] = c;
    matrix[j][i] = 0;
    return 0;
  }

  if (a == 2){
    matrix[i][j] = 0;
    matrix[j][i] = c;
    return 0;
  }

  if (a == 3){
    matrix[i][j] = c;
    matrix[j][i] = c;
    return 0;
  }

  return -1;
}
int check_matrix_size(double **matrix){
  int size = -1;
  int i,j;
  int max_j;
  int max_i;

  max_i = -1;
  for(i=0; matrix[i]; i++){
    max_i = i;

    max_j = -1;
    for(j=0; !isinf(matrix[i][j]); j++){
//      printw("%d:%f ", j, matrix[i][j]);
      max_j = j;
    }
    if (max_j == -1){
      return -1;
    }
    if(size == -1){
      size = max_j;
    }else if (size != max_j){
      return -2;
    }
  }
  if (max_i != size){
    printw("%d %d\n", max_i, size);
    getch();
    return -3;
  }
  return size+1;
}

double** resize_matrix(double** matrix, int new_size){
  double ** res = matrix;;
  int i,j;

  for(i=0; i<new_size; i++){
    if (!res[i]){
      res = realloc(res, (i+2)*sizeof(double*));
      res[i] = malloc((new_size+1)*sizeof(double));
      for(j=0; j<new_size; j++){
        res[i][j] = NAN;
      }
      res[i][i] = 1.0;
      res[i][new_size] = INFINITY;
    }else{
      for(j=0; j<new_size; j++){
        if (isinf(res[i][j])){
          res[i] = realloc(res[i], (j+2)*sizeof(double));
          res[i][j] = NAN;
          res[i][j+1] = INFINITY;
        }
      }
    }
  }
  return res;
}

int main(){
  int i,j;
  double c;
  double **matrix_corr;
  wchar_t** pokaz;
  double **data;
  int size;

  setlocale(LC_ALL, "");

  pokaz = load_pokazat("pokazat.txt");
  if (!pokaz){
    return -1;
  }
  for(i=0; i<M; i++){
    if (!pokaz[i]){
      M = i;
      break;
    }
  }

  setlocale(LC_ALL, "C");

  data = load_data("data.txt");
  if (!data){
    destroy_array((void**)pokaz);
    return -1;
  }
  for(i=0; i<M; i++){
    if (!data[i]){
      M = i;
      break;
    }
    for(j=0; j<N; j++){
      if (isinf(data[i][j])){
//        printf("N=>%d\n", i);
        N = j;
        break;
      }
    }
  }
  matrix_corr = load_data("corr_alpha.txt");


  setlocale(LC_ALL, "");

  initscr();
  if (matrix_corr){
//    printw("has matrix\n");
    size = check_matrix_size(matrix_corr);
  }else{
//    printw("no matrix\n");
    size = 0;
  }
//  printf("M=%d N=%d\n", M,N);
//  return 0;

  if (size <= 0){
    if (size == -1){
    addwstr(L"Есть пустая строка\n");
    } else if (size == -2){
    addwstr(L"Длина строк разная\n");
    } else if (size == -3){
    addwstr(L"Матрица не квадратная\n");
    }
    if (matrix_corr){
      destroy_array((void**)matrix_corr);
    }
    matrix_corr = 0;
  }

  if (size < 2){
    size = 2;
  }
  askparams(&M, size, M, &N, 2, N);

  if (!matrix_corr){
    matrix_corr = create_matrix(M);
  }else{
    matrix_corr = resize_matrix(matrix_corr, M);
  }

  for(i=0; i<M; i++){
    for(j=i+1; j<M; j++){
//      printw("%d %d =1> %f\n", i,j,matrix_corr[i][j]);
      if (!isnan(matrix_corr[i][j]) || isinf(matrix_corr[i][j])){
        continue;
      }
//      printw("%d %d =2> %f\n", i,j,matrix_corr[i][j]);
      c = calc_corr(data[i], data[j], N);
      if (!isnan(c) && fabs(c) > alpha){
        if (update_arc(matrix_corr, (const wchar_t**)pokaz, i, j, c) < 0){
          i = M;
          break;
        }
      }
    }
  }

  endwin();

  setlocale(LC_ALL, "C");
  save_correlation("corr_alpha.txt", matrix_corr, M);
  save_adjustment("adj_matr.txt", matrix_corr, M);
  destroy_matrix(matrix_corr, M);
  destroy_array((void**)pokaz);
  destroy_array((void**)data);
  return 0;
}
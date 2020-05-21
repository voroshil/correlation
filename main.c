#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <math.h>
#include <panel.h>

// Величина корреляции, приравнянная к 0
#define THR 1e-6
// Максимальная длина имени показателя
#define MAX_STR 1000


#define ADJ_YES 1
#define ADJ_NO  0
#define ADJ_UNK 2
#define ADJ_END 255

void messagebox(const wchar_t *str){
  const int height = 6;
  const int width = 40;
  const wchar_t *title = L"Сообщение";
  const wchar_t *footer = L"Нажмите ENTER";
  int starty;
  int startx;
  int len;
  int flen;
  int fpos;
  WINDOW* local_win;
  WINDOW* internal_win;
  char c;

  starty = (LINES - height) / 2;
  startx = (COLS - width) / 2;

  len = wcslen(title);
  if (len > width -6)
    len = width -6;

  flen = wcslen(footer);
  if (flen > width - 6)
    flen = width - 6;

  fpos = (width - flen) / 2;

  refresh();

  local_win = newwin(height, width, starty, startx);

  box(local_win, 0, 0);
  mvwaddch(local_win, 0, 2, ACS_RTEE);
  mvwaddwstr(local_win, 0, 3, title);
  mvwaddch(local_win, 0, 3+len, ACS_LTEE);

  mvwaddch(local_win, height-1, fpos-1, ACS_RTEE);
  mvwaddwstr(local_win, height-1, fpos, footer);
  mvwaddch(local_win, height-1, fpos + flen, ACS_LTEE);

  internal_win = derwin(local_win, height-2, width-2, 1,1);
  mvwaddwstr(internal_win, 0, 0, str);

  wrefresh(local_win);
  wrefresh(internal_win);

  do{
   c = getch();
  }while (c!= '\n');


  wborder(local_win, ' ',' ',' ',' ',' ',' ',' ',' ');
  wrefresh(local_win);
  delwin(internal_win);
  delwin(local_win);
  redrawwin(stdscr);
  refresh();

}

void ask_int_param(const wchar_t *str, int *p, int min, int max){
  do{
    addwstr(str);
    printw("[%d-%d]: ", min, max);
    if(min < max){
      scanw("%d", p);
    }else{
      *p = min;
      printw("%d\n", *p);
    }
    if (*p < min || *p > max){
      addwstr(L"Значение введено неверно\n");
    }
  }while(*p < min || *p > max);
}
void ask_double_param(const wchar_t *str, double *p, double min, double max){
  do{
    addwstr(str);
    printw("[%.1f-%.1f]: ", min, max);
    scanw("%f", p);
    if (*p < min || *p > max){
      addwstr(L"Значение введено неверно\n");
    }
  }while(*p < min || *p > max);
}

void askparams(int *pm, int m_min, int m_max, int *pn, int n_min, int n_max, double* palpha, double a_min, double a_max){

  ask_int_param(L"Число показателей m ", pm, m_min, m_max);
  ask_int_param(L"Число лет n ", pn, n_min, n_max);
  ask_double_param(L"alpha ", palpha, a_min, a_max);

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
        if (loaded >= max_size){
          max_size = max_size + 10;
          buf = realloc(buf, max_size * sizeof(wchar_t*));
        }
        len = wcslen(str);
        if (len > MAX_STR - 1)
          len = MAX_STR - 1;

        buf[loaded] = malloc((len+1) * sizeof(wchar_t));
        wcsncpy(buf[loaded], str, len+1);
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

unsigned char* load_adj_line(FILE* f){
  unsigned char* buf;
  int loaded, max_size;
  char c;

  max_size = 100;
  loaded = 0;

  buf = malloc(max_size * sizeof(unsigned char));
  buf[0] = ADJ_END;

  do{
    c = fgetc(f);

    switch(c){
      case '0':
        buf[loaded] = ADJ_NO;
        break;
      case '1':
        buf[loaded] = ADJ_YES;
        break;
      case '.':
        buf[loaded] = ADJ_UNK;
        break;
      case ' ':
      case '\n':
      case EOF:
        loaded++;
        if(loaded >= max_size-1){
          max_size = max_size + 100;
          buf = realloc(buf, max_size * sizeof(unsigned char));
        }
        buf[loaded] = ADJ_END;
    }
  }while(c!= '\n' && c != EOF);
//  printf("%d\n",loaded);
  buf = realloc(buf, (loaded+1)*sizeof(unsigned char));
  buf[loaded] = ADJ_END;
  return buf;
}
unsigned char** load_adj_data(char* filename){
   unsigned char** buf;
   unsigned char* row;
   int max_size, len, i;
   int loaded;
   FILE *f;


   max_size = 1;
   loaded = 0;

   buf = malloc(max_size * sizeof(unsigned char*));
   f= fopen(filename, "r");
   if (!f)
     return 0;
   do{
      row = load_adj_line(f);
      len = 0;
      for(i=0; row[i] != ADJ_END; i++){
        len = i+1;
      }
      if (len > 0){
        if (loaded >= max_size){
          max_size = max_size + 1;
          buf = realloc(buf, max_size * sizeof(unsigned char*));
        }
        buf[loaded] = row;
        loaded++;
//        printf("lines: %d\n", loaded);
      }
   }while(len > 0);

   fclose(f);

   buf = realloc(buf, (loaded+1) * sizeof(unsigned char*));
   buf[loaded] = 0;
   return buf;
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
  WINDOW* w1;
  WINDOW* w2;

//  mvprintw(2, 3, "%d => 1, %d => 2", i+1, j+1);
//  mvaddwstr(3, 3, L"1 - ");
//  mvaddwstr(5, 3, L"2 - ");
  mvaddwstr(7, 3, L"Коэффициент корреляции:        ");
  mvprintw(7, 27, "%f", c);
  mvaddwstr(9, 3, L"Выберите один из вариантов:");
  mvaddwstr(11, 3, L"1. Показатель 1->2 (1 влияет на 2, то есть 2 зависит от 1)");
  mvaddwstr(12, 3, L"2. Показатель 2->1 (2 влияет на 1, то есть 1 зависит от 2)");
  mvaddwstr(13, 3, L"3. Показатель 1<->2 (зависят друг от друга)");
  mvaddwstr(14, 3, L"0. Сохранить и выйти");

  w2 = newwin(4, 10, 3, 3);
  mvwprintw(w2, 0, 0, "1 - [%d] ", i+1);
  mvwprintw(w2, 2, 0, "2 - [%d] ", j+1);
  wrefresh(w2);

  w1 = newwin(4, COLS-14, 3, 14);
  mvwaddwstr(w1, 0, 0, str1);
  mvwaddwstr(w1, 2, 0, str2);
  wrefresh(w1);

  refresh();

  while (a < 0 || a > 3){
    mvaddwstr(16, 3, L"Введите число 1,2,3 или 0:       ");
    redrawwin(w1);
    redrawwin(w2);
    wrefresh(w1);
    wrefresh(w2);
    refresh();
    mvscanw(16,30, "%d", &a);
  }

  delwin(w1);
  delwin(w2);
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
void save_correlation(char* filename, unsigned char** adj,double** matrix, int len){
  int i,j;
  FILE * f;

  f = fopen(filename, "w");
  for(i=0; i<len; i++){
    for(j=0; j<len; j++){
      if (adj[i][j] == ADJ_YES){
        fprintf(f, "%f", fabs(matrix[i][j]));
      }else if (adj[i][j] == ADJ_NO){
        fprintf(f, "%f", 0.0);
      }else{
        fprintf(f, "%f", NAN);
      }
      if (j < len-1){
        fprintf(f, " ");
      }else{
        fprintf(f, "\n");
      }
    }
  }
  fclose(f);
}
void save_adjustment(char* filename, unsigned char** matrix, int len){
  int i,j;
  FILE *f;
  double c;

  f = fopen(filename, "w");
  for(i=0; i<len; i++){
    for(j=0; j<len; j++){
      c = matrix[i][j];
      if (c == ADJ_YES)
        fprintf(f, "1");
      else if (c == ADJ_NO)
        fprintf(f, "0");
      else
        fprintf(f, ".");
      if(matrix[i][j+1] != ADJ_END)
        fprintf(f, " ");
    }
    fprintf(f, "\n");
  }
  fclose(f);
}
int update_arc(unsigned char** matrix, const wchar_t** pokaz, int i, int j, double c){
  int a;

  do{
    a = display(pokaz, i, j, c);
  }while (a < 0 || a > 3);

  if (a == 1){
    matrix[i][j] = ADJ_YES;
    matrix[j][i] = ADJ_NO;
    return 0;
  }

  if (a == 2){
    matrix[i][j] = ADJ_NO;
    matrix[j][i] = ADJ_YES;
    return 0;
  }

  if (a == 3){
    matrix[i][j] = ADJ_YES;
    matrix[j][i] = ADJ_YES;
    return 0;
  }

  return -1;
}

int check_adj_matrix_size(unsigned char **matrix){
  int size = -1;
  int i,j;
  int max_j;
  int max_i;

  max_i = -1;
  for(i=0; matrix[i]; i++){
    max_i = i;

    max_j = -1;
    for(j=0; matrix[i][j] != ADJ_END; j++){
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
//    printw("%d %d\n", max_i, size);
//    getch();
    return -3;
  }
  return size+1;
}

unsigned char** resize_adj_matrix(unsigned char** matrix, int new_size){
  unsigned char ** res = matrix;;
  int i,j;

  res = realloc(res, (new_size+1)*sizeof(unsigned char*));
  i=0; 
  while(res[i]) {
    res[i] = realloc(res[i], (new_size+1)*sizeof(unsigned char));
    j=0;
    while(res[i][j] != ADJ_END) {
      j++;
    }
    while(j<new_size){
      res[i][j] = ADJ_UNK;
      j++;
    }
    res[i][new_size] = ADJ_END;
    i++;
  }
  while(i < new_size){
    res[i] = malloc((new_size+1)*sizeof(unsigned char));
    for(j=0; j<new_size; j++){
      res[i][j] = ADJ_UNK;
    }
    res[i][new_size] = ADJ_END;
    i++;
  }
  for(i=0; i<new_size; i++){
    res[i][i] = ADJ_YES;
  }
  return res;
}
double** resize_matrix(double** matrix, int new_size){
  double ** res = matrix;;
  int i,j;

  res = realloc(res, (new_size+1)*sizeof(double));
  i=0; 
  while(res[i]) {
    res[i] = realloc(res[i], (new_size+1)*sizeof(double));
    j=0;
    while(!isinf(res[i][j])) {
      j++;
    }
    while(j<new_size){
      res[i][j] = NAN;
      j++;
    }
    res[i][new_size] = INFINITY;
    i++;
  }
  while(i < new_size){
    res[i] = malloc((new_size+1)*sizeof(double));
    for(j=0; j<new_size; j++){
      res[i][j] = NAN;
    }
    res[i][new_size] = INFINITY;
    i++;
  }
  for(i=0; i<new_size; i++){
    res[i][i] = 1.0;
  }
  return res;
}

int main(){
  int i,j;
  double c;
  double **matrix_corr;
  unsigned char**matrix_adj;
  wchar_t** pokaz;
  double **data;
  int size;

  // Количество показателей
  int M = 10000;
  // С какого показателя начинать
  int first_M = 1;
  // Количество лет статистики
  int N = 10000;
  // Пороговый коэффициент корреляции
  double alpha = 0;


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
  matrix_adj = load_adj_data("adj_matr.txt");

  setlocale(LC_ALL, "");

  initscr();

  if (matrix_adj){
    size = check_adj_matrix_size(matrix_adj);

    messagebox(L"Найдены ранее введенные данные в adj_matrix.txt. Заполнение будет продолжено.");

  }else{
    size = 0;
  }


  if (size <= 0){
    if (size == -1){
    messagebox(L"Есть пустая строка");
    } else if (size == -2){
    messagebox(L"Длина строк разная");
    } else if (size == -3){
    messagebox(L"Матрица не квадратная");
    }
    if (!matrix_adj){
      matrix_adj = malloc(sizeof(unsigned char*));
      matrix_adj[0] = 0;
    }
  }

  for(i=0; i<size; i++){
    if (matrix_adj[i]){
      for(j=i+1; j<size;j++){
        if (matrix_adj[i][j] == ADJ_UNK){
          first_M=i+1;
          i=size;
          break;
        }else if (matrix_adj[i][j] == ADJ_END){
          break;
        }
      }
    }
  }

  if (size < 2){
    size = 2;
  }
  ask_int_param(L"Число показателей m ", &M, size, M);
  if (M > 1){
    ask_int_param(L"Начать с показателя под номером ", &first_M, first_M, M-1);
  }else{
    first_M = 1;
  }
  ask_int_param(L"Число лет n ", &N, 2, N);
  ask_double_param(L"alpha ", &alpha, 0, 1);

  erase();

  matrix_adj = resize_adj_matrix(matrix_adj, M);
  matrix_corr = create_matrix(M);
  for(i=0; i<M; i++){
    matrix_corr[i][i] = 1.0;
    for(j=i+1; j<M; j++){
      c = calc_corr(data[i], data[j], N);
      matrix_corr[i][j] = c;
      matrix_corr[j][i] = c;
    }
  }

  for(i=first_M-1; i<M; i++){
    for(j=i+1; j<M; j++){
      c = matrix_corr[i][j];
      if (isnan(c) || fabs(c) <= alpha){
          matrix_adj[i][j] = 0;
          matrix_adj[j][i] = 0;
      }
      if (matrix_adj[i][j] == 2 || matrix_adj[j][i] == 2){
          if (update_arc(matrix_adj, (const wchar_t**)pokaz, i, j, c) < 0){
            i = M;
            break;
          }
      }
    }
  }

  endwin();

  setlocale(LC_ALL, "C");
  save_correlation("corr_alpha.txt", matrix_adj, matrix_corr, M);
  save_adjustment("adj_matr.txt", matrix_adj, M);
  destroy_matrix(matrix_corr, M);
  destroy_array((void**)pokaz);
  destroy_array((void**)data);
  return 0;
}
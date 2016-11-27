
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <signal.h>
#include <string.h> 
#include <pthread.h>
#include <sys/types.h>
#include <time.h>

#define DELAY 50000
#define LABY_WIDTH 29 // Columnas
#define LABY_HEIGHT 31 // Filas
#define LABY_OFFSET 4
#define RAND_MAX 32
#define SCORE_SIZE 4
#define CLOCKS_PER_SEC 1000000


typedef struct{
   char name;
   char keyst;
   int x,y,ax,ay,lives;
} player;

void *myTime(void *);
pthread_t thread, thread_id;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

const // Laberinto, Espacios representan caminos, caracteres especiales representan paredes, w's es la casa intermedia
char maze[LABY_HEIGHT][LABY_WIDTH] = {
	{'.','-','-','-','-','-','-','-','-','-','-','-','-','`','.','-','-','-','-','-','-','-','-','-','-','-','-','`'},
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'},
	{'|',' ','.','-','-','`',' ','.','-','-','-','`',' ','|','|',' ','.','-','-','-','`',' ','.','-','-','`',' ','|'},
	{'|','p','|','x','x','|',' ','|','x','x','x','|',' ','|','|',' ','|','x','x','x','|',' ','|','x','x','|','p','|'},
	{'|',' ','[','-','-',']',' ','[','-','-','-',']',' ','[',']',' ','[','-','-','-',']',' ','[','-','-',']',' ','|'},
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'},
	{'|',' ','.','-','-','`',' ','.','`',' ','.','-','-','-','-','-','-','`',' ','.','`',' ','.','-','-','`',' ','|'},
	{'|',' ','[','-','-',']',' ','|','|',' ','[','-','-','`','.','-','-',']',' ','|','|',' ','[','-','-',']',' ','|'},
	{'|',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ','|'},
	{'[','-','-','-','-','`',' ','|','[','-','-','`',' ','|','|',' ','.','-','-',']','|',' ','.','-','-','-','-',']'},
	{'x','x','x','x','x','|',' ','|','.','-','-',']',' ','[',']',' ','[','-','-','`','|',' ','|','x','x','x','x','x'},
	{'x','x','x','x','x','|',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ','|','x','x','x','x','x'},
	{'x','x','x','x','x','|',' ','|','|',' ','.','-','t','f','f','r','-','`',' ','|','|',' ','|','x','x','x','x','x'},
	{'q','-','-','-','-',']',' ','[',']',' ','|','f','f','f','f','f','f','|',' ','[',']',' ','[','-','-','-','-','y'},
	{'o','w','w','w','w','w',' ',' ',' ',' ','|','f','f','f','f','f','f','|',' ',' ',' ',' ','w','w','w','w','w','i'},
	{'r','-','-','-','-','`',' ','.','`',' ','|','f','f','f','f','f','f','|',' ','.','`',' ','.','-','-','-','-','t'},
	{'x','x','x','x','x','|',' ','|','|',' ','[','-','-','-','-','-','-',']',' ','|','|',' ','|','x','x','x','x','x'},
	{'x','x','x','x','x','|',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ','|','x','x','x','x','x'},
	{'x','x','x','x','x','|',' ','|','|',' ','.','-','-','-','-','-','-','`',' ','|','|',' ','|','x','x','x','x','x'},
	{'.','-','-','-','-',']',' ','[',']',' ','[','-','-','`','.','-','-',']',' ','[',']',' ','[','-','-','-','-','`'},
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'},
	{'|',' ','.','-','-','`',' ','.','-','-','-','`',' ','|','|',' ','.','-','-','-','`',' ','.','-','-','`',' ','|'},
	{'|',' ','[','-','`','|',' ','[','-','-','-',']',' ','[',']',' ','[','-','-','-',']',' ','|','.','-',']',' ','|'},
	{'|',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ','w','w',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ','|'},
	{'[','-','`',' ','|','|',' ','.','`',' ','.','-','-','-','-','-','-','`',' ','.','`',' ','|','|',' ','.','-',']'},
	{'.','-',']',' ','[',']',' ','|','|',' ','[','-','-','`','.','-','-',']',' ','|','|',' ','[',']',' ','[','-','`'},
	{'|',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ','|'},
	{'|','p','.','-','-','-','-',']','[','-','-','`',' ','|','|',' ','.','-','-',']','[','-','-','-','-','`','p','|'},
	{'|',' ','[','-','-','-','-','-','-','-','-',']',' ','[',']',' ','[','-','-','-','-','-','-','-','-',']',' ','|'},
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'},
	{'[','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',']'}
};

  wint_t pacsprite[4] = {'ᗢ','ᗣ','ᗤ','ᗧ'};
  
  wint_t ghosprite[2] = {'ᗝ','ᗜ'};
  
  char cur_pac_sprite = '0';
  char cur_gho_sprite = '0';
  char cur_maze[LABY_HEIGHT][LABY_WIDTH];
  char col_maze[LABY_HEIGHT][LABY_WIDTH];
  
  int parent_x, parent_y;
  long uno, msec;
  clock_t diff;
  char key;
  
  
  
  
  int game_st = 1,pills = 0,pp = 0, pu = 0;
  
  
  
void *myTime (void *ptr) {
}

void spawn_fruit () {
	srand(time(NULL));
	int r = rand();    //returns a pseudo-random integer between 0 and RAND_MAX
	
	//if (r = 0) 
}

void init_laby(){
	int x, y;
	
	for (y = 0; y <= LABY_HEIGHT-1 ; y++){
		for (x = 0; x <= LABY_WIDTH-2; x++){
		
			switch (maze[y][x]){
				case ' ': col_maze[y][x] = 0; cur_maze[y][x] = 0; pills++; break;
				case 'p': col_maze[y][x] = 0; cur_maze[y][x] = 0; break;
                case 'r': col_maze[y][x] = 0; break;
                case 'y': col_maze[y][x] = 0; break;
                case 'w': col_maze[y][x] = 0; break;
                case 'f': col_maze[y][x] = 1; break;
				default:  col_maze[y][x] = -1;
			}
		}
	}
	
}

int y_in_laby (int y) {
	return y+LABY_OFFSET;
}

int x_in_laby (WINDOW *screen, int x) {
	int xm, ym;
	getmaxyx(screen, ym, xm);
	return xm/2 - LABY_WIDTH/2 + x;
}

int draw_laby(WINDOW *screen){
	int x, y, xm, ym, i;
	getmaxyx(screen, ym, xm);
	
	if (xm < LABY_WIDTH || ym < LABY_HEIGHT) { // Verifica si la pantalla de terminal es mas pequeña que el laberinto
		mvwprintw(screen, 3, 3, "La pantalla es muy pequeña, por favor agrandar");
		return 1;
	}
	
	wattron(screen, COLOR_PAIR(3));
	
	for (y = 0; y <= LABY_HEIGHT-1 ; y++){
		for (x = 0; x <= LABY_WIDTH-2; x++){
		
			switch (maze[y][x]){
				case ' ': wattron(screen, COLOR_PAIR(2)); mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "▪"); wattron(screen, COLOR_PAIR(3)); break;
				case 'p': wattron(screen, COLOR_PAIR(2)); mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "⦿"); wattron(screen, COLOR_PAIR(3)); break;
				case '-': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "═"); break;
				case '|': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "║"); break;
				case '.': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╔"); break;
				case ']': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╝"); break;
				case '`': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╗"); break;
				case '[': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╚"); break;
				case 't': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╕"); break;
				case 'q': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╘"); break;
				case 'r': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╒"); break;
				case 'y': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "╛"); break;
				case 'w': // Camino sin pildoras
				case 'o': // Portal izquierdo
				case 'i': // Portal derecho
				case 'f': // Solo fantasma puede pasar
				case 'x': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , " "); break; // Out of bound
				default:  mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "E");
			}
		}
	}
	wattron(screen, COLOR_PAIR(1));
	return 0;
}

void control (player ix[5],int i){
	switch (ix[i].keyst){
      case('A'): ix[i].ax = 0; ix[i].ay = -1; break;
      case('B'): ix[i].ax = 0; ix[i].ay = 1; break;
      case('C'): ix[i].ax = 1; ix[i].ay = 0; break;
      case('D'): ix[i].ax = -1; ix[i].ay = 0; break;
      default: ix[i].ax = 0; ix[i].ay = 0;
    }
}

int pla_dir(player ix[5], int i){
	
	static int last[5];
	
    if     ( ix[i].ax == 0  && ix[i].ay == 1 ) {last[i] = 1; return 1;} 
    else if( ix[i].ax == 0  && ix[i].ay == -1) {last[i] = 2; return 2;}
    else if( ix[i].ax == 1  && ix[i].ay == 0 ) {last[i] = 3; return 3;}
    else if( ix[i].ax == -1 && ix[i].ay == 0 ) {last[i] = 4; return 4;}
	else return last[i];
	
return 0;
}

int collition (player ix[5], int i) {
	
	switch (pla_dir(ix,i)){
		case 1: if (col_maze[ix[i].y+1][ix[i].x] == 0) {return 0;} else if (col_maze[ix[i].x][ix[i].y-1] == 1 && !i) {return 0;} else return 1; break;
		case 2: if (col_maze[ix[i].y-1][ix[i].x] == 0) {return 0;} else if (col_maze[ix[i].x][ix[i].y+1] == 1 && !i) {return 0;} else return 1; break;
		case 3: if (col_maze[ix[i].y][ix[i].x+1] == 0) {return 0;} else if (col_maze[ix[i].x+1][ix[i].y] == 1 && !i) {return 0;} else return 1; break;
		case 4: if (col_maze[ix[i].y][ix[i].x-1] == 0) {return 0;} else if (col_maze[ix[i].x-1][ix[i].y] == 1 && !i) {return 0;} else return 1; break;
		default: return 1;
	}	
}

void movement (player ix[5], int i) {
	
		ix[i].x = ix[i].x + ix[i].ax;
        ix[i].y = ix[i].y + ix[i].ay;	
        //ix[i].ax = 0; ix[i].ay = 0;
}

int print_pac(player ix[5], WINDOW *screen, int status) {
	
	wattron(screen, COLOR_PAIR(2));
	int set = pla_dir(ix,0);
	switch (set){
		case 1: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗣ" ); break;
        case 2: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗢ" ); break;
        case 3: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗧ" ); break;
        case 4: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗤ" ); break;
        
        default: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "E" ); break;
	}
	wattron(screen, COLOR_PAIR(1));
	return 0;
}

int print_ghost(player ix[5],int i, WINDOW *screen){
	
	for (int y = i;y>0;y--){
		if (!pp) { wattron(screen, COLOR_PAIR(y+3)); mvwprintw(screen, y_in_laby(ix[y].y), x_in_laby(screen, ix[y].x), "ᗝ" ); wattron(screen, COLOR_PAIR(1));}
		else { wattron(screen, COLOR_PAIR(1)); mvwprintw(screen, y_in_laby(ix[y].y), x_in_laby(screen, ix[y].x), "ᗝ" );}
	}
	return 0;
}

void collition_pac (player ix[5], int i, WINDOW *screen) {
    static int pp_start;
    
    for (int y = 1; y < i + 1; y++){
        if (ix[0].x == ix[y].x && ix[0].y == ix[y].y) {mvwprintw(screen, 15, 2, "tocado");}
    }
    
    if (pp) {
        
    }
    else    {
        
    }
}

void draw_borders(WINDOW *screen) {
  int x, y, i;

  getmaxyx(screen, y, x);

  // 4 corners
  mvwprintw(screen, 0, 0, "+");
  mvwprintw(screen, y - 1, 0, "+");
  mvwprintw(screen, 0, x - 1, "+");
  mvwprintw(screen, y - 1, x - 1, "+");

  // sides
  for (i = 1; i < (y - 1); i++) {
    mvwprintw(screen, i, 0, "|");
    mvwprintw(screen, i, x - 1, "|");
  }

  // top and bottom
  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, 0, i, "-");
    mvwprintw(screen, y - 1, i, "-");
  }
}

void resize (int parent_y, int parent_x, WINDOW *field, WINDOW *score) {
	
	int new_x, new_y;
	
	getmaxyx(stdscr, new_y, new_x);
	
	if (new_y != parent_y || new_x != parent_x) {
	
	  parent_x = new_x;
      parent_y = new_y;

      wresize(field, new_y - SCORE_SIZE, new_x);
      wresize(score, SCORE_SIZE, new_x);
      mvwin(score, new_y - SCORE_SIZE, 0);

      wclear(stdscr);
      wclear(field);
      wclear(score);

      //draw_borders(field);
	  draw_laby(field);
      draw_borders(score);
	}
}

void init_players (player jugadores[5]) {
  jugadores[0].name = 'pacman';
  jugadores[0].keyst = 'A';
  jugadores[0].x = 1;
  jugadores[0].y = 1;
  jugadores[0].ax = 1;
  jugadores[0].ay = 0;
  
  jugadores[1].name = 'Blinky';
  jugadores[1].keyst = 'A';
  jugadores[1].x = 9;
  jugadores[1].y = 15;
  jugadores[1].ax = 0;
  jugadores[1].ay = 0;
  
  jugadores[2].name = 'Pinky';
  jugadores[2].keyst = 'A';
  jugadores[2].x = 13;
  jugadores[2].y = 14;
  jugadores[2].ax = 0;
  jugadores[2].ay = 0;
  
  jugadores[3].name = 'Inky';
  jugadores[3].keyst = 'A';
  jugadores[3].x = 14;
  jugadores[3].y = 15;
  jugadores[3].ax = 0;
  jugadores[3].ay = 0;
  
  jugadores[4].name = 'Clyde';
  jugadores[4].keyst = 'A';
  jugadores[4].x = 15;
  jugadores[4].y = 14;
  jugadores[4].ax = 0;
  jugadores[4].ay = 0;
}



void server (player jugadores[5]) {
    
}


int menu_cursor (int i, char y) {
    switch (y) {
      case('A'): if (i == 0) {return 3;} else {return i-1;};
      case('B'): if (i == 3) {return 0;} else {return i+1;};
      default: return i;
    }
}


int menu (WINDOW *screen, int jugador) {
    
    int cursor = 0;
    char w_cursor = ' ';
    
    do {
        
        wclear(screen);
    mvwprintw (screen, y_in_laby(0), x_in_laby(screen, 5), "Pᗧc Mᗣn");
    
    
     mvwprintw (screen, y_in_laby(2), x_in_laby(screen, 3), "Iniciar Partida");
     mvwprintw (screen, y_in_laby(4), x_in_laby(screen, 3), "Unirse a Partida");
     mvwprintw (screen, y_in_laby(6), x_in_laby(screen, 3), "Instrucciones");
     mvwprintw (screen, y_in_laby(8), x_in_laby(screen, 3), "Salir");
     
     w_cursor = wgetch(screen);
     if (w_cursor == '\r') {break;}
     cursor = menu_cursor(cursor, w_cursor);
     w_cursor = ' ';
     
     switch (cursor){
         case 0:
             mvwprintw (screen, y_in_laby(2), x_in_laby(screen, 0), "➤");
             break;
         case 1:
             mvwprintw (screen, y_in_laby(4), x_in_laby(screen, 0), "➤");
             break;
         case 2:
             mvwprintw (screen, y_in_laby(6), x_in_laby(screen, 0), "➤");
             break;
         case 3:
             mvwprintw (screen, y_in_laby(8), x_in_laby(screen, 0), "➤");
             break;
    }
     
    wrefresh(screen);
    
    } while (1);
}

void game_loop (player jugadores[5], int jugador, clock_t start, WINDOW *field, WINDOW *score) {
    
        
	resize(parent_y, parent_x, field, score);

    key = wgetch(field);
    if (key != ERR){jugadores[jugador].keyst = key;}
	if (jugadores[jugador].keyst == 'p'){game_st = 0;}
	
	usleep(DELAY);
	
	diff = clock() - start;
	msec = (diff + DELAY) * 1000 / CLOCKS_PER_SEC;
	uno = msec%1000/50;
	
	

    
    wclear(stdscr);
     wclear(field);
     wclear(score);
    // draw to our windows
	draw_laby(field);
    mvwprintw(field, 1, 1, "No es Pacman");
	wattron(score, COLOR_PAIR(2)); mvwprintw(score, 2, 1, "ᗧ"); wattron(score, COLOR_PAIR(1)); mvwprintw(score, 2, 3, "x"); wattron(score, COLOR_PAIR(2)); mvwprintw(score, 2, 5, "3"); wattron(score, COLOR_PAIR(1));
    
	draw_borders(score);
	mvwprintw (score, 1, 2, "%d", msec);
	
	mvwprintw (field, 3, 2, "%d", pla_dir(jugadores,jugador));
	mvwprintw (field, 5, 2, &jugadores[jugador].keyst);
	
	mvwprintw (field, 7, 2, "%d", jugadores[jugador].x);
	mvwprintw (field, 9, 2, "%d", jugadores[jugador].y);
	
	//mvwprintw(field, jugadores[0].y, jugadores[0].x, "M" );
	control(jugadores,0);
	
	if (!collition(jugadores,jugador)) {movement(jugadores,jugador);};
    collition_pac (jugadores, 4, field);
	print_pac(jugadores,field,0);
	print_ghost(jugadores,4,field);
	

    
	//movement (&jugadores, 0);

    // refresh each window
    wrefresh(field);
	wrefresh(score);
    
}

void init_ncurses () {
    
    setlocale(LC_ALL, "");
    (void) initscr();      // ncurses 
    keypad(stdscr, TRUE);  // Habilita teclado 
    (void) nonl();         // No usar nl 
    (void) cbreak();       // No esperar por \n 
    (void) noecho();         // no echo input 
    curs_set(FALSE);
	(void) start_color();  // Iniciar color 
  
  init_pair(1, COLOR_WHITE,  COLOR_BLACK); //Default
  init_pair(2, COLOR_YELLOW, COLOR_BLACK); //Laberinto
  init_pair(3, COLOR_BLUE,   COLOR_BLACK); //Pacman y pildoras
  init_pair(4, COLOR_RED,  COLOR_BLACK); // Inny
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Miney
  init_pair(6, COLOR_GREEN,   COLOR_BLACK); // Minney
  init_pair(7, COLOR_CYAN,  COLOR_BLACK); // Clide 
}











int main(int argc, char *argv[]) {
  int jugador = 0;
  clock_t start = clock();
  


  player jugadores[5];
  
  pthread_create( &thread, NULL, myTime, NULL);
  
  init_ncurses();


  init_players(jugadores);
  
  
  getmaxyx(stdscr, parent_y, parent_x);
  
  WINDOW *w = initscr();;
  WINDOW *field = newwin(parent_y - SCORE_SIZE, parent_x, 0, 0);
  WINDOW *score = newwin(SCORE_SIZE, parent_x, parent_y - SCORE_SIZE, 0);
  
  menu(field, jugador);
  //getch();
  //sleep(1);


  noraw();
  cbreak();
  nodelay(stdscr,TRUE);
  nodelay(w, TRUE);
  nodelay(field, TRUE);
  nodelay(score, TRUE);
  timeout(0);
  wtimeout(field, 0);
  
  
  init_laby();
  
  
  draw_borders(score);
  
    wrefresh(score);
    mvwprintw(score, 1, 1, "Puntuacion");

	
 while(jugadores[jugador].keyst != 'q') {game_loop(jugadores, jugador, start, field, score);}
	 
  game_st = 0;
  
  endwin();
  printf ("%d \n", uno);
 
  exit(0);

  return 0;
}


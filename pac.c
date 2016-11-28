
#include <ncurses.h>    // Interfaz TUI para consola
#include <stdio.h>      // Metodos basicos de I/O
#include <unistd.h>     // Entorno UNIX
#include <time.h>       // Metodos de tiempo
#include <stdlib.h>     // Metodos estandares
#include <wchar.h>      // Uso de caracteres UNICODE UTF-8
#include <locale.h>     // Gestion de codiifcacion de caracteres para la consola
#include <signal.h>     // Control de signals
#include <string.h>     // Metodos de Strings
#include <pthread.h>    // Medodos de Hilos

#include <sys/types.h>  // Tipos de variables adicionales 
#include<sys/socket.h>  // Metodos para Sockets
#include<netinet/in.h>  // Metodos para Protocolos de internet
#include <semaphore.h>  // Control por semaforos
#include <errno.h>      // Metodos para numeros de error


#define DELAY 50000
#define LABY_WIDTH 29 // Columnas
#define LABY_HEIGHT 31 // Filas
#define LABY_OFFSET 4
#define RAND_MAX 32
#define SCORE_SIZE 4
#define CLOCKS_PER_SEC 1000000


// Sendables things here

typedef struct{
   char name;
   char keyst;
   signed char x,y,ax,ay,death;
   unsigned char lives;
} player;



typedef struct{
   player jugadores[5];
   unsigned char col_maze[LABY_HEIGHT][LABY_WIDTH];
   signed char cur_maze[LABY_HEIGHT][LABY_WIDTH];
   unsigned char game_st,pills,pp, pu;
}__attribute__ ((packed, aligned(2)))  package;




unsigned char col_maze[LABY_HEIGHT][LABY_WIDTH];
signed char cur_maze[LABY_HEIGHT][LABY_WIDTH];
unsigned char game_st = 1,pp = 0, pu = 0;
unsigned short pills = 0;
unsigned long uno, msec;
int xo,xi,yo,yi;


// Sendable things end

void *myTime(void *);
pthread_t thread, thread_id;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

const // Laberinto, Espacios representan caminos, caracteres especiales representan paredes, f's es la casa intermedia
char maze[LABY_HEIGHT][LABY_WIDTH] = {                                                                                 //x
	{'.','-','-','-','-','-','-','-','-','-','-','-','-','`','.','-','-','-','-','-','-','-','-','-','-','-','-','`'}, //1
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'}, //2
	{'|',' ','.','-','-','`',' ','.','-','-','-','`',' ','|','|',' ','.','-','-','-','`',' ','.','-','-','`',' ','|'}, //3
	{'|','p','|','x','x','|',' ','|','x','x','x','|',' ','|','|',' ','|','x','x','x','|',' ','|','x','x','|','p','|'}, //4
	{'|',' ','[','-','-',']',' ','[','-','-','-',']',' ','[',']',' ','[','-','-','-',']',' ','[','-','-',']',' ','|'}, //5
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'}, //6
	{'|',' ','.','-','-','`',' ','.','`',' ','.','-','-','-','-','-','-','`',' ','.','`',' ','.','-','-','`',' ','|'}, //7
	{'|',' ','[','-','-',']',' ','|','|',' ','[','-','-','`','.','-','-',']',' ','|','|',' ','[','-','-',']',' ','|'}, //8
	{'|',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ','|'}, //9
	{'[','-','-','-','-','`',' ','|','[','-','-','`',' ','|','|',' ','.','-','-',']','|',' ','.','-','-','-','-',']'}, //10
	{'x','x','x','x','x','|',' ','|','.','-','-',']',' ','[',']',' ','[','-','-','`','|',' ','|','x','x','x','x','x'}, //11
	{'x','x','x','x','x','|',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ','|','x','x','x','x','x'}, //12
	{'x','x','x','x','x','|',' ','|','|',' ','.','-','t','f','f','r','-','`',' ','|','|',' ','|','x','x','x','x','x'}, //13
	{'q','-','-','-','-',']',' ','[',']',' ','|','x','x','w','w','x','x','|',' ','[',']',' ','[','-','-','-','-','y'}, //14
	{'o','w','w','w','w','w',' ',' ',' ',' ','|','x','x','w','w','x','x','|',' ',' ',' ',' ','w','w','w','w','w','i'}, //15
	{'r','-','-','-','-','`',' ','.','`',' ','|','x','x','w','w','x','x','|',' ','.','`',' ','.','-','-','-','-','t'}, //16
	{'x','x','x','x','x','|',' ','|','|',' ','[','-','-','-','-','-','-',']',' ','|','|',' ','|','x','x','x','x','x'}, //17
	{'x','x','x','x','x','|',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ','|','x','x','x','x','x'}, //18
	{'x','x','x','x','x','|',' ','|','|',' ','.','-','-','-','-','-','-','`',' ','|','|',' ','|','x','x','x','x','x'}, //19
	{'.','-','-','-','-',']',' ','[',']',' ','[','-','-','`','.','-','-',']',' ','[',']',' ','[','-','-','-','-','`'}, //20
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'}, //21
	{'|',' ','.','-','-','`',' ','.','-','-','-','`',' ','|','|',' ','.','-','-','-','`',' ','.','-','-','`',' ','|'}, //22
	{'|',' ','[','-','`','|',' ','[','-','-','-',']',' ','[',']',' ','[','-','-','-',']',' ','|','.','-',']',' ','|'}, //23
	{'|',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ',' ','w','w',' ',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ','|'}, //24
	{'[','-','`',' ','|','|',' ','.','`',' ','.','-','-','-','-','-','-','`',' ','.','`',' ','|','|',' ','.','-',']'}, //25
	{'.','-',']',' ','[',']',' ','|','|',' ','[','-','-','`','.','-','-',']',' ','|','|',' ','[',']',' ','[','-','`'}, //26
	{'|',' ',' ',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ','|','|',' ',' ',' ',' ',' ',' ','|'}, //27
	{'|','p','.','-','-','-','-',']','[','-','-','`',' ','|','|',' ','.','-','-',']','[','-','-','-','-','`','p','|'}, //28
	{'|',' ','[','-','-','-','-','-','-','-','-',']',' ','[',']',' ','[','-','-','-','-','-','-','-','-',']',' ','|'}, //29
	{'|',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','|'}, //30
	{'[','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',']'}  //31
//y   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28
};


  signed short parent_x, parent_y;
  clock_t diff;
  char key;
  
  
  
  

  
  
  
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
				case 'p': col_maze[y][x] = 0; cur_maze[y][x] = 1; break;
                case 'o': col_maze[y][x] = 0; cur_maze[y][x] = -1; xo = x; yo = y; break;
                case 'i': col_maze[y][x] = 0; cur_maze[y][x] = -1; xi = x; yi = y; break;
                case 'y': col_maze[y][x] = 0; cur_maze[y][x] = -1; break;
                case 'w': col_maze[y][x] = 0; cur_maze[y][x] = -1; break;
                case 'f': col_maze[y][x] = 1; cur_maze[y][x] = -1; break;
				default:  col_maze[y][x] = -1; cur_maze[y][x] = -1;
			}
		}
	}
	
}

signed short y_in_laby (short y) {
	return y+LABY_OFFSET;
}

signed short x_in_laby (WINDOW *screen, short x) {
	int xm, ym;
	getmaxyx(screen, ym, xm);
	return xm/2 - LABY_WIDTH/2 + x;
}

signed char draw_laby(WINDOW *screen){
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
                case ' ': break;
				case 'p': break;
				case 'w': // Camino sin pildoras
				case 'o': // Portal izquierdo
				case 'i': // Portal derecho
				case 'f': // Solo fantasma puede pasar
				case 'x': mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , " "); break; // Out of bound
				default:  mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "E");
			}
			
			switch (cur_maze[y][x]) {
                case 0:  wattron(screen, COLOR_PAIR(2)); mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "▪"); wattron(screen, COLOR_PAIR(3)); break;
				case 1:  wattron(screen, COLOR_PAIR(2)); mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , "⦿"); wattron(screen, COLOR_PAIR(3)); break;
                case -1: break;
                case -2: mvwprintw(screen, y_in_laby(y),x_in_laby(screen, x) , " "); break;
                default: break;
            }
		}
	}
	wattron(screen, COLOR_PAIR(1));
	return 0;
}



signed char pla_dir(player ix[5], char i){
	
	static int last[5];
	
    if     ( ix[i].ax == 0  && ix[i].ay == 1 ) {last[i] = 1; return 1;} 
    else if( ix[i].ax == 0  && ix[i].ay == -1) {last[i] = 2; return 2;}
    else if( ix[i].ax == 1  && ix[i].ay == 0 ) {last[i] = 3; return 3;}
    else if( ix[i].ax == -1 && ix[i].ay == 0 ) {last[i] = 4; return 4;}
	else return last[i];
	
return 0;
}

signed char collition (player ix[5], char i, char f) {

int dir;
if (f>0) {dir = f;} else {dir = pla_dir(ix,i);}; 

switch (dir){
		case 1: if (col_maze[ix[i].y+1][ix[i].x] == 0) {return 0;} else if (col_maze[ix[i].y+1][ix[i].x] == 1 && i != 0) {return 0;} else return 1; break;
		case 2: if (col_maze[ix[i].y-1][ix[i].x] == 0) {return 0;} else if (col_maze[ix[i].y-1][ix[i].x] == 1 && i != 0) {return 0;} else return 1; break;
		case 3: if (col_maze[ix[i].y][ix[i].x+1] == 0) {return 0;} else if (col_maze[ix[i].y][ix[i].x+1] == 1 && i != 0) {return 0;} else return 1; break;
		case 4: if (col_maze[ix[i].y][ix[i].x-1] == 0) {return 0;} else if (col_maze[ix[i].y][ix[i].x-1] == 1 && i != 0) {return 0;} else return 1; break;
		default: return 1;
	}
}

void control (player ix[5],char i){

    static int dir;

	switch (ix[i].keyst){
      case('A'): if (!collition(ix,i,dir = 2)) {ix[i].ax = 0; ix[i].ay = -1;} break;
      case('B'): if (!collition(ix,i,dir = 1)) {ix[i].ax = 0; ix[i].ay = 1;  dir = 1;} break;
      case('C'): if (!collition(ix,i,dir = 3)) {ix[i].ax = 1; ix[i].ay = 0;  dir = 2;} break;
      case('D'): if (!collition(ix,i,dir = 4)) {ix[i].ax = -1; ix[i].ay = 0; dir = 3;} break;
      //default: ix[i].ax = 0; ix[i].ay = 0;
    }
}

void movement (player ix[5], char i) {
	
		ix[i].x = ix[i].x + ix[i].ax;
        ix[i].y = ix[i].y + ix[i].ay;
        
        if (ix[i].x == xo && ix[i].y == yo) {ix[i].x = xi-1 ;ix[i].y = yi;}
        if (ix[i].x == xi && ix[i].y == yi) {ix[i].x = xo+1 ;ix[i].y = yo;}
    
        //ix[i].ax = 0; ix[i].ay = 0;
}

void print_pac(player ix[5], WINDOW *screen, int status) {
	
	wattron(screen, COLOR_PAIR(2));
	int set = pla_dir(ix,0);
    
    if (ix[0].death || pu == 2) {mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), " " );}
    else {
    
        switch (set){
            case 1: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗣ" ); break;
            case 2: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗢ" ); break;
            case 3: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗧ" ); break;
            case 4: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗤ" ); break;
        
            default: mvwprintw(screen, y_in_laby(ix[0].y), x_in_laby(screen, ix[0].x), "ᗧ" ); break;
        }
    }
	wattron(screen, COLOR_PAIR(1));
}

void print_ghost(player ix[5],int i, WINDOW *screen){
	
	for (int y = i;y>0;y--){
		if (!pp) { wattron(screen, COLOR_PAIR(y+3)); mvwprintw(screen, y_in_laby(ix[y].y), x_in_laby(screen, ix[y].x), "ᗝ" ); wattron(screen, COLOR_PAIR(1));}
		
		else if (!ix[i].death) {mvwprintw(screen, y_in_laby(ix[y].y), x_in_laby(screen, ix[y].x), " " );}
        else { wattron(screen, COLOR_PAIR(1)); mvwprintw(screen, y_in_laby(ix[y].y), x_in_laby(screen, ix[y].x), "ᗝ" );}    
	}
}

void collition_pac (player ix[5], int i, WINDOW *screen) {
    static int pp_start;
    
    if (cur_maze[ix[0].y][ix[0].x] == 0) {cur_maze[ix[0].y][ix[0].x] = -2; pills--;};
    
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
  jugadores[0].keyst = ' ';
  jugadores[0].x = 1;
  jugadores[0].y = 1;
  jugadores[0].ax = 0;
  jugadores[0].ay = 0;
  jugadores[0].death = 0;
  
  jugadores[1].name = 'Blinky';
  jugadores[1].keyst = ' ';
  jugadores[1].x = 13;
  jugadores[1].y = 13;
  jugadores[1].ax = 0;
  jugadores[1].ay = 0;
  jugadores[1].death = 0;
  
  jugadores[2].name = 'Pinky';
  jugadores[2].keyst = ' ';
  jugadores[2].x = 14;
  jugadores[2].y = 13;
  jugadores[2].ax = 0;
  jugadores[2].ay = 0;
  jugadores[2].death = 0;
  
  jugadores[3].name = 'Inky';
  jugadores[3].keyst = ' ';
  jugadores[3].x = 13;
  jugadores[3].y = 14;
  jugadores[3].ax = 0;
  jugadores[3].ay = 0;
  jugadores[3].death = 0;
  
  jugadores[4].name = 'Clyde';
  jugadores[4].keyst = ' ';
  jugadores[4].x = 14;
  jugadores[4].y = 14;
  jugadores[4].ax = 0;
  jugadores[4].ay = 0;
  jugadores[4].death = 0;
}

/**

void server (player jugadores[5]) {
    
}


signed char cliente (char jugador) {
    int sockfd,portno,n;
  	struct pak paquete; 
    	paquete.x1=5;
    	paquete.y1=0;
    	paquete.reloj=1;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char mensaje[256];
    	int semilla = 1;

	if(argc < 3){
		fprintf(stderr,"error leyendo el mensajeeeee %s \n",argv[1]);
		return 1;	
	}
	label:
	usleep (1000000) ;
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd < 0){
		fprintf(stderr,"error leyendo el mensaje\n");
		return 1;	
	}
	server = gethostbyname(argv[1]);

	if(server == NULL){
		fprintf(stderr,"error leyendo el mensaje\n");
		return 1;		
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);

	serv_addr.sin_port = htons(portno);
	
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		fprintf(stderr,"error de conexion\n");
		return 1;
	}
    	printf("ya conecte\n");

    	read(sockfd,&paquete,sizeof(struct pak));
    	printf("ya lei mensaje\n");
    	int i,j;
	
	//-- control del movimiento en el tablero------

    	for(i=0;i<MAX;i++){
        	for(j=0;j<MAX;j++){
            		printf("%d",paquete.buffer[i][j]);
        	}
        	printf("\n");
    	}

	//-- reloj recibido por socket --
    	printf("reloj actualizado:%d\n",paquete.reloj);

	//---- Validacion de limites ---
		printf("%d\n",paquete.x1);
		printf("%d\n",paquete.y1);
		if(paquete.y1==14){
			paquete.y1=0;
		}
   	if(paquete.x1<15 && paquete.y1<15){
		    paquete.buffer[paquete.y1][paquete.x1] = 0;
        	paquete.y1++;
        	//paquete.reloj++;
		write(sockfd,&paquete,sizeof(struct pak));
        }

	close(sockfd);
    	goto label;
	return 0;
}

**/

signed char menu_cursor (char i, char y) {
    switch (y) {
      case('A'): if (i == 0) {return 3;} else {return i-1;};
      case('B'): if (i == 3) {return 0;} else {return i+1;};
      default: return i;
    }
}


signed char menu (WINDOW *screen, char jugador) {
    
    signed char cursor = 0;
    char w_cursor = ' ';
    
    do {
        
        wclear(screen);
    mvwprintw (screen, y_in_laby(0), x_in_laby(screen, 5), "Pᗧc Mᗣn");
    
    
     mvwprintw (screen, y_in_laby(2), x_in_laby(screen, 3), "Iniciar Partida");
     mvwprintw (screen, y_in_laby(4), x_in_laby(screen, 3), "Unirse a Partida");
     mvwprintw (screen, y_in_laby(6), x_in_laby(screen, 3), "Instrucciones");
     mvwprintw (screen, y_in_laby(8), x_in_laby(screen, 3), "Salir");
     
     mvwprintw (screen, y_in_laby(3), x_in_laby(screen, 3), "M");
     
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
    
     w_cursor = wgetch(screen);
     if (w_cursor == '\r') {break;}
     cursor = menu_cursor(cursor, w_cursor);
     w_cursor = ' ';
     
    wrefresh(screen);
    
    } while (1);
return cursor;
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
	mvwprintw (field, 5, 2, "%d", pills);
	
	mvwprintw (field, 7, 2, "%d", jugadores[jugador].x);
	mvwprintw (field, 9, 2, "%d", jugadores[jugador].y);
	
	//mvwprintw(field, jugadores[0].y, jugadores[0].x, "M" );
	
	control(jugadores,jugador);
	if (!collition(jugadores,jugador,0)) {movement(jugadores,jugador);};
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
  char jugador = 2;
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


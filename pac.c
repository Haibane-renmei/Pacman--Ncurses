
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

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>


#define DELAY 50000
#define LABY_WIDTH 29 // Columnas
#define LABY_HEIGHT 31 // Filas
#define LABY_OFFSET 4
#define RAND_MAX 32
#define SCORE_SIZE 4
#define CLOCKS_PER_SEC 1000000

#define PAC_LAG   100000
#define GHO_LAG_1 PAC_LAG * 1.2
#define GHO_LAG_2 PAC_LAG * 1.3
#define GHO_LAG_3 PAC_LAG * 1.5
#define GHO_LAG_4 PAC_LAG * 1.7

#define PORT_SERVER_1 5001
#define PORT_SERVER_2 5002
#define PORT_SERVER_3 5003
#define PORT_SERVER_4 5004
#define PORT_SERVER_5 5005

typedef struct{
   signed char name;
   signed char keyst;
   signed char x,y,ax,ay,death;
   unsigned char lives;
} player;


struct __attribute__((packed, aligned(2))) package{
   player jugadores[5];
   unsigned char col_maze[LABY_HEIGHT][LABY_WIDTH];
   signed char cur_maze[LABY_HEIGHT][LABY_WIDTH];
   unsigned char game_st, pp, pu;
};

sem_t tablero;


struct package paquete;

unsigned char col_maze[LABY_HEIGHT][LABY_WIDTH];
signed char cur_maze[LABY_HEIGHT][LABY_WIDTH];
unsigned char game_st = 0, pp = 0, pu = 0, server = 1;
unsigned short pills = 0;
unsigned long uno, msec;
signed int xo,xi,yo,yi;
clock_t start, diff;
const signed char maze[LABY_HEIGHT][LABY_WIDTH];
pthread_t serv1, serv2, serv3, serv4, serv5, mov0, mov1, mov2, mov3, mov4, movid0, movid1, movid2, movid3, movid4;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
signed short parent_x, parent_y;
signed char key;


const signed // Laberinto, Espacios representan caminos, caracteres especiales representan paredes, f's es la casa intermedia
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


//@sn numero de servidor
void *server_fantasma(void *sn){
	
	int a = (int)sn;
	int port;
	switch(a)//se asigna un puerto
	{
	case 1:
		port=PORT_SERVER_1;
    	break;
	case 2:
    	port=PORT_SERVER_2;
    	break;
	case 3:
    	port=PORT_SERVER_3;
    	break;
	case 4:
    	port=PORT_SERVER_4;
    	break;
	}
	
	int sockfd,newsockfd,portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr,cli_addr;
	int n;// cantidad de caracteres leidos
	char move;


	sockfd = socket(AF_INET,SOCK_STREAM,0); //declaracion de estructura 
	//parametros (familia,puerto de asignacion,parametro de configuracion)
	if(sockfd < 0){
		fprintf(stderr,"error abriendo socket server %d \n",a);
		return NULL;	
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));//inicializando en 0
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	portno = port; // asignacion de puerto 
	serv_addr.sin_port = htons(portno); 

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		fprintf(stderr,"error de conexion al socket server %d \n",a);
		return NULL;	
	}
	//segundo parametro, la cantidad de procesos concurrentes permitidos
	while(game_st == 0){
		listen(sockfd,5); // espera bloqueado por una respuesta 
	
		clilen = sizeof(cli_addr);

		newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
		if(newsockfd < 0){
			fprintf(stderr,"error en conexion entrante server %d \n",a);
			return NULL;
		}
	
		sem_wait(&tablero);//blokea semaforo
		//escribir algo

		paquete.jugadores[1].name='a';

    	sem_post(&tablero);


		n = write(newsockfd,&paquete,sizeof( struct package));
		if(n < 0){
			fprintf(stderr,"error escribiendo el mensaje 1 server %d \n",a);
			return NULL;
		}

		n = read(newsockfd,&move,sizeof(char));
		if(n < 0){
			fprintf(stderr,"error leyendo el mensaje 2 server %d \n",a);
			return NULL;
		}
		printf("char: %c \n",move);
	

   		printf("Peticion Realizada server 1 server %d \n",a);
	
	} //fin del while
	close(newsockfd);
	close(sockfd);
	
	return NULL;


}


void *server_pacman(void *arg){
	
	int sockfd,newsockfd,portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr,cli_addr;
	int n;// cantidad de caracteres leidos
	int a = 5; 


	sockfd = socket(AF_INET,SOCK_STREAM,0); //declaracion de estructura 
	//parametros (familia,puerto de asignacion,parametro de configuracion)
	if(sockfd < 0){
		fprintf(stderr,"error abriendo socket server %d \n",a);
		return NULL;	
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));//inicializando en 0
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	portno = PORT_SERVER_5; // asignacion de puerto 
	serv_addr.sin_port = htons(portno); 

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		fprintf(stderr,"error de conexion al socket server %d \n",a);
		return NULL;	
	}
	//segundo parametro, la cantidad de procesos concurrentes permitidos
	while(game_st == 0){
		listen(sockfd,5); // espera bloqueado por una respuesta 
	
		clilen = sizeof(cli_addr);

		newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
		if(newsockfd < 0){
			fprintf(stderr,"error en conexion entrante server %d \n",a);
			return NULL;
		}
	
		sem_wait(&tablero);//blokea semaforo
		//escribir algo

		paquete.jugadores[1].name='a';

    	sem_post(&tablero);

		//envia el estado del juego
		n = write(newsockfd,&paquete,sizeof( struct package));
		if(n < 0){
			fprintf(stderr,"error escribiendo el mensaje 1 server %d \n",a);
			return NULL;
		}
		//leer movimiento
		n = read(newsockfd,&move,sizeof(char));
		if(n < 0){
			fprintf(stderr,"error leyendo el mensaje 2 server %d \n",a);
			return NULL;
		}
	
		printf("char: %c \n",move);
		

   		printf("Peticion Realizada server 1 server %d \n",a);
	
	}
	close(newsockfd);
	close(sockfd);
	
	return NULL;


}


void spawn_fruit () {
	srand(time(NULL));
	int r = rand();    //returns a pseudo-random integer between 0 and RAND_MAX
	
	//if (r = 0) 
}


signed short y_in_laby (signed short y) {
	return y+LABY_OFFSET;
}

signed short x_in_laby (WINDOW *screen, short x) {
	int xm, ym;
	getmaxyx(screen, ym, xm);
	return xm/2 - LABY_WIDTH/2 + x;
}

signed char pla_dir(char i){
	
	static int last[5];
	
    if     ( paquete.jugadores[i].ax == 0  && paquete.jugadores[i].ay == 1 ) {last[i] = 1; return 1;} 
    else if( paquete.jugadores[i].ax == 0  && paquete.jugadores[i].ay == -1) {last[i] = 2; return 2;}
    else if( paquete.jugadores[i].ax == 1  && paquete.jugadores[i].ay == 0 ) {last[i] = 3; return 3;}
    else if( paquete.jugadores[i].ax == -1 && paquete.jugadores[i].ay == 0 ) {last[i] = 4; return 4;}
	else return last[i];
	
return 0;
}

signed char collition (char i, char f) {

int dir;
if (f>0) {dir = f;} else {dir = pla_dir(i);}; 

switch (dir){
		case 1: if (col_maze[paquete.jugadores[i].y+1][paquete.jugadores[i].x] == 0) {return 0;} else if (col_maze[paquete.jugadores[i].y+1][paquete.jugadores[i].x] == 1 && i != 0) {return 0;} else return 1; break;
		case 2: if (col_maze[paquete.jugadores[i].y-1][paquete.jugadores[i].x] == 0) {return 0;} else if (col_maze[paquete.jugadores[i].y-1][paquete.jugadores[i].x] == 1 && i != 0) {return 0;} else return 1; break;
		case 3: if (col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x+1] == 0) {return 0;} else if (col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x+1] == 1 && i != 0) {return 0;} else return 1; break;
		case 4: if (col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x-1] == 0) {return 0;} else if (col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x-1] == 1 && i != 0) {return 0;} else return 1; break;
		default: return 1;
	}
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

void control (char i){

    static int dir;
if (paquete.jugadores[i].death != 1) {
	switch (paquete.jugadores[i].keyst){
      case('A'): if (!collition(i,dir = 2)) {paquete.jugadores[i].ax = 0; paquete.jugadores[i].ay = -1;} break;
      case('B'): if (!collition(i,dir = 1)) {paquete.jugadores[i].ax = 0; paquete.jugadores[i].ay = 1;  dir = 1;} break;
      case('C'): if (!collition(i,dir = 3)) {paquete.jugadores[i].ax = 1; paquete.jugadores[i].ay = 0;  dir = 2;} break;
      case('D'): if (!collition(i,dir = 4)) {paquete.jugadores[i].ax = -1; paquete.jugadores[i].ay = 0; dir = 3;} break;
      //default: paquete.jugadores[i].ax = 0; paquete.jugadores[i].ay = 0;
    }
}
}

void control_cli (char i){

    static int dir;
if (paquete.jugadores[i].death != 1) {
	switch (paquete.jugadores[i].keyst){
      case('A'): return 0;
      case('B'): return 1;
      case('C'): return 2;
      case('D'): return 3;
      //default: paquete.jugadores[i].ax = 0; paquete.jugadores[i].ay = 0;
    }
}
}

void *movimiento(void *y){
    
    int i = (int)y;
    int delay;
    
    switch (i) {
        case 0: delay = PAC_LAG  ; break;
        case 1: delay = GHO_LAG_1; break;
        case 2: delay = GHO_LAG_2; break;
        case 3: delay = GHO_LAG_3; break;
        case 4: delay = GHO_LAG_4; break;
        default: delay = 0;
    }
    
    while (game_st == 0) {
        
        if (!collition(i,0)) {
        
        usleep(delay);
        
        paquete.jugadores[i].x = paquete.jugadores[i].x + paquete.jugadores[i].ax;
        paquete.jugadores[i].y = paquete.jugadores[i].y + paquete.jugadores[i].ay;
        
        if (paquete.jugadores[i].x == xo && paquete.jugadores[i].y == yo) {paquete.jugadores[i].x = xi-1 ;paquete.jugadores[i].y = yi;}
        if (paquete.jugadores[i].x == xi && paquete.jugadores[i].y == yi) {paquete.jugadores[i].x = xo+1 ;paquete.jugadores[i].y = yo;}
    
        }
    }
pthread_exit(NULL);
}

void print_pac(WINDOW *screen, int status) {
	
	wattron(screen, COLOR_PAIR(2));
	int set = pla_dir(0);
    
    if (paquete.jugadores[0].death || pu == 2) {mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), " " );}
    else {
    
        switch (set){
            case 1: mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), "ᗣ" ); break;
            case 2: mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), "ᗢ" ); break;
            case 3: mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), "ᗧ" ); break;
            case 4: mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), "ᗤ" ); break;
        
            default: mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), "ᗧ" ); break;
        }
    }
	wattron(screen, COLOR_PAIR(1));
}

void print_ghost(int i, WINDOW *screen){
	
    int y;
	for (y = i;y>0;y--){
		if (!pp && paquete.jugadores[y].death == 0) { wattron(screen, COLOR_PAIR(y+3)); mvwprintw(screen, y_in_laby(paquete.jugadores[y].y), x_in_laby(screen, paquete.jugadores[y].x), "ᗝ" ); wattron(screen, COLOR_PAIR(1));}
		
		else if (pp && paquete.jugadores[y].death == 0) { wattron(screen, COLOR_PAIR(1)); mvwprintw(screen, y_in_laby(paquete.jugadores[y].y), x_in_laby(screen, paquete.jugadores[y].x), "ᗝ" );}
		
        else if (paquete.jugadores[y].death == 1) {mvwprintw(screen, y_in_laby(paquete.jugadores[y].y), x_in_laby(screen, paquete.jugadores[y].x), " " );}    
	}
}

void collition_pac (WINDOW *screen) {
    static int pp_start;
    static unsigned long end;
    int y;
    
    if (cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] == 0) {cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] = -2; pills--;};
    if (cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] == 1) {cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] = -2; pp = 1; end = msec/1000 + 20;};
    if (end == msec/1000) {pp = 0;};
    mvwprintw(screen, 16, 2, "%d", end);
    
    for (y = 1; y < 5; y++){
        if ( (paquete.jugadores[0].x == paquete.jugadores[y].x && paquete.jugadores[0].y == paquete.jugadores[y].y) && pp == 0) {paquete.jugadores[0].death = 1; paquete.jugadores[0].lives--;}
        if ( (paquete.jugadores[0].x == paquete.jugadores[y].x && paquete.jugadores[0].y == paquete.jugadores[y].y) && pp == 1) {paquete.jugadores[y].death = 1;}
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

void init_players (void) {
  paquete.jugadores[0].name = 'pacman';
  paquete.jugadores[0].keyst = ' ';
  paquete.jugadores[0].x = 1;
  paquete.jugadores[0].y = 1;
  paquete.jugadores[0].ax = 0;
  paquete.jugadores[0].ay = 0;
  paquete.jugadores[0].death = 0;
  paquete.jugadores[0].lives= 3;
  
  paquete.jugadores[1].name = 'Blinky';
  paquete.jugadores[1].keyst = ' ';
  paquete.jugadores[1].x = 13;
  paquete.jugadores[1].y = 13;
  paquete.jugadores[1].ax = 0;
  paquete.jugadores[1].ay = 0;
  paquete.jugadores[1].death = 0;
  
  paquete.jugadores[2].name = 'Pinky';
  paquete.jugadores[2].keyst = ' ';
  paquete.jugadores[2].x = 14;
  paquete.jugadores[2].y = 13;
  paquete.jugadores[2].ax = 0;
  paquete.jugadores[2].ay = 0;
  paquete.jugadores[2].death = 0;
  
  paquete.jugadores[3].name = 'Inky';
  paquete.jugadores[3].keyst = ' ';
  paquete.jugadores[3].x = 13;
  paquete.jugadores[3].y = 14;
  paquete.jugadores[3].ax = 0;
  paquete.jugadores[3].ay = 0;
  paquete.jugadores[3].death = 0;
  
  paquete.jugadores[4].name = 'Clyde';
  paquete.jugadores[4].keyst = ' ';
  paquete.jugadores[4].x = 14;
  paquete.jugadores[4].y = 14;
  paquete.jugadores[4].ax = 0;
  paquete.jugadores[4].ay = 0;
  paquete.jugadores[4].death = 0;
}

void restore_player (int i) {
switch (i) {
    case 0:
  paquete.jugadores[0].name = 'pacman';
  paquete.jugadores[0].keyst = ' ';
  paquete.jugadores[0].x = 1;
  paquete.jugadores[0].y = 1;
  paquete.jugadores[0].ax = 0;
  paquete.jugadores[0].ay = 0;
  break;
  
    case 1:
  paquete.jugadores[1].name = 'Blinky';
  paquete.jugadores[1].keyst = ' ';
  paquete.jugadores[1].x = 13;
  paquete.jugadores[1].y = 13;
  paquete.jugadores[1].ax = 0;
  paquete.jugadores[1].ay = 0;
  break;
  
    case 2:
  paquete.jugadores[2].name = 'Pinky';
  paquete.jugadores[2].keyst = ' ';
  paquete.jugadores[2].x = 14;
  paquete.jugadores[2].y = 13;
  paquete.jugadores[2].ax = 0;
  paquete.jugadores[2].ay = 0;
  break;
  
    case 3:
  paquete.jugadores[3].name = 'Inky';
  paquete.jugadores[3].keyst = ' ';
  paquete.jugadores[3].x = 13;
  paquete.jugadores[3].y = 14;
  paquete.jugadores[3].ax = 0;
  paquete.jugadores[3].ay = 0;
  break;
  
    case 4:
  paquete.jugadores[4].name = 'Clyde';
  paquete.jugadores[4].keyst = ' ';
  paquete.jugadores[4].x = 14;
  paquete.jugadores[4].y = 14;
  paquete.jugadores[4].ax = 0;
  paquete.jugadores[4].ay = 0;
  break;
}
}

void death_manager (WINDOW *screen) {
    static unsigned long end[5];
    unsigned int i;
    
  for (i = 0; i < 5; i++) {  
  mvwprintw(screen, 17 + i, 2, "%d" ,paquete.jugadores[i].death);
  if (paquete.jugadores[i].death == 1 && end[i] == 0) {end[i] = msec/1000 + 20; restore_player(i);}
  if (paquete.jugadores[i].death == 1 && end[i] == msec/1000) {paquete.jugadores[i].death = 0; end[i] = 0;}
  }
}

void draw_laby(WINDOW *screen, WINDOW *score){
	int x, y, xm, ym, i;
	getmaxyx(screen, ym, xm);
	
	if (xm < LABY_WIDTH || ym < LABY_HEIGHT) { // Verifica si la pantalla de terminal es mas pequeña que el laberinto
		mvwprintw(screen, 3, 3, "La pantalla es muy pequeña, por favor agrandar");
		return 1;
	}
	
	wattron(score, COLOR_PAIR(2)); mvwprintw(score, 2, 1, "ᗧ"); wattron(score, COLOR_PAIR(1)); mvwprintw(score, 2, 3, "x"); 
    
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
}

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

int game_loop (int jugador, WINDOW *field, WINDOW *score) {
    
while (game_st == 0) {

    death_manager(field);
	resize(parent_y, parent_x, field, score);

    key = wgetch(field);
    if (key != ERR) paquete.jugadores[jugador].keyst = key;
    
    if (game_st == 1) return -1;
    if (pills == 0) return 0;
    if (paquete.jugadores[0].lives == 0) return 1;
	if (paquete.jugadores[jugador].keyst == 'q') return 2;
	
	usleep(DELAY);
	
	diff = clock() - start;
	msec = (diff + DELAY) * 1000 / CLOCKS_PER_SEC;
	uno = msec%1000/50;
	
	

    
    wclear(stdscr);
     wclear(field);
     wclear(score);
    // draw to our windows
	draw_laby(field, score);
	wattron(score, COLOR_PAIR(2)); mvwprintw(score, 2, 5, "%d",paquete.jugadores[0].lives); wattron(score, COLOR_PAIR(1));

	draw_borders(score);
	mvwprintw (score, 1, 2, "%d", msec/1000);
	
	//mvwprintw(field, jugadores[0].y, jugadores[0].x, "M" );
	
    if (server = 1) {
        control(jugador);
        control(2);
        collition_pac (field);
    } else {
        control_cli(1);
    }
    
	print_pac(field,0);
	print_ghost(4,field);
    wrefresh(field);
	wrefresh(score);
}
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
  char jugador = 0;
  
  int i = 0;
  
 start = clock();
  
  
  init_ncurses();
  init_players();
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
  
  if (server == 1) {
   pthread_create(&serv1, NULL , server_fantasma , (void*)1);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv2, NULL , server_fantasma , (void*)2);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv3, NULL , server_fantasma , (void*)3);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv4, NULL , server_fantasma , (void*)4);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv5, NULL , server_pacman , NULL);
    }
  
  
  
  init_laby();
  
  
  draw_borders(score);
  
    wrefresh(score);
    mvwprintw(score, 1, 1, "Puntuacion");
    
      pthread_create(&mov0, NULL, movimiento, (void*)0);
      pthread_create(&mov1, NULL, movimiento, (void*)1);
      pthread_create(&mov2, NULL, movimiento, (void*)2);
      pthread_create(&mov3, NULL, movimiento, (void*)3);
      pthread_create(&mov4, NULL, movimiento, (void*)4);

  switch (game_loop(jugador, field, score)) {
      case 0:
          mvwprintw(field, 4, 4, "Pacman a Ganado");
          mvwprintw(field, 5, 4, "Gracias por jugar");
          break;
      case 1:
          mvwprintw(field, 4, 4, "Pacman a Perdido");
          mvwprintw(field, 5, 4, "Gracias por jugar");
          break;
      case 2:
          mvwprintw(field, 5, 4, "Has salido de la partida");
          break;
      case -1:
          mvwprintw(field, 5, 4, "La partida a sido cerrada");
          break;
}
  wrefresh(field);

  game_st = 0;
  sleep(2);
  endwin();
  
  
   pthread_join(&serv1, NULL);//espera el fianl de hilos
   pthread_join(&serv2, NULL);
   pthread_join(&serv3, NULL);
   pthread_join(&serv4, NULL);
   pthread_join(&serv5, NULL);	
  
  exit(0);

  return 0;
}


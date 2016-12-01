
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
#include <netdb.h>      // Medodos de Hilos

#include <sys/types.h>  // Tipos de variables adicionales 
#include<sys/socket.h>  // Metodos para Sockets
#include<netinet/in.h>  // Metodos para Protocolos de internet
#include <semaphore.h>  // Control por semaforos
#include <errno.h>      // Metodos para numeros de error

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>


#define DELAY 50000
#define MAX 15
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
   unsigned char game_st, pp, pu, pu_c;
   unsigned short pills;
   unsigned long msec;
   
};

sem_t tablero;


struct package paquete;

unsigned char col_maze[LABY_HEIGHT][LABY_WIDTH];
signed char cur_maze[LABY_HEIGHT][LABY_WIDTH];
signed char errco = 0; jugador = -1; keyst_local;
unsigned char server = 1;
const char *ip;
clock_t start;
signed int xo,xi,yo,yi, port;
clock_t diff;
const signed char maze[LABY_HEIGHT][LABY_WIDTH];
pthread_t serv1, serv2, serv3, serv4, serv5, cli, mov0, mov1, mov2, mov3, mov4, movid0, movid1, movid2, movid3, movid4;
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


char get_ip(void){//genera el ip de la maquina 
	int fd;
 	struct ifreq ifr;

 	fd = socket(AF_INET, SOCK_DGRAM, 0);

 	/* I want to get an IPv4 IP address */
 	ifr.ifr_addr.sa_family = AF_INET;

 	/* I want IP address attached to "eth0" */
 	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 	ioctl(fd, SIOCGIFADDR, &ifr);

 	close(fd);
    
    strncpy(fd, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), IFNAMSIZ-1);

 	/* display result */
 	return fd;

}

int client () {
    int sockfd,portno,n;
	char move; 
	struct sockaddr_in serv_addr;
    struct in_addr addr;
	struct hostent *servidor;
    
    switch(jugador)//se asigna un puerto
	{
    case 0:
		portno=PORT_SERVER_5;
    	break;
	case 1:
		portno=PORT_SERVER_1;
    	break;
	case 2:
    	portno=PORT_SERVER_2;
    	break;
	case 3:
    	portno=PORT_SERVER_3;
    	break;
	case 4:
    	portno=PORT_SERVER_4;
    	break;
	}
	
    
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd < 0){
		return 1;
	}
	
	//inet_pton(AF_INET, "127.0.0.1", &addr);
	//servidor = gethostbyaddr(&addr, sizeof(addr), AF_INET);

	servidor = gethostbyname("127.0.0.1");
	if(servidor == NULL){
		errco = 2;
		return 1;
	}
	errco = 0;

	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char*)servidor->h_addr,(char*)&serv_addr.sin_addr.s_addr,servidor->h_length);

	serv_addr.sin_port = htons(portno);
    
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		errco = 1;
		return 2;
	}

    	read(sockfd,&paquete,sizeof(struct package));
        errco = 0;

	write(sockfd,&keyst_local,sizeof(char));
        

close(sockfd);
	return 0;
}

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
		return NULL;	
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));//inicializando en 0
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	portno = port; // asignacion de puerto 
	serv_addr.sin_port = htons(portno); 

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        errco = 2;
		return NULL;	
	}
	//segundo parametro, la cantidad de procesos concurrentes permitidos
	while(paquete.game_st == 0){
        
        
		listen(sockfd,5); // espera bloqueado por una respuesta 
	
		clilen = sizeof(cli_addr);

		newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
		if(newsockfd < 0){
			errco = 1;
			return NULL;
		}
		errco = 0;
	
		//sem_wait(&tablero);//blokea semaforo
    	//sem_post(&tablero);


		n = write(newsockfd,&paquete,sizeof( struct package));
		if(n < 0){
			errco = 3;
			return NULL;
		}

		if (a != jugador) {n = read(newsockfd,&paquete.jugadores[a].keyst,sizeof(char));
            if(n < 0){
			errco = 4;
			return NULL;
		}
		control(a);
        };
		
        errco = 0;

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
	char move;


	sockfd = socket(AF_INET,SOCK_STREAM,0); //declaracion de estructura 
	//parametros (familia,puerto de asignacion,parametro de configuracion)
	if(sockfd < 0){
		errco = 1;
		return NULL;	
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));//inicializando en 0
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	portno = PORT_SERVER_5; // asignacion de puerto 
	serv_addr.sin_port = htons(portno); 

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		errco = 2;
		return NULL;	
	}
	//segundo parametro, la cantidad de procesos concurrentes permitidos
	while(paquete.game_st == 0){
		listen(sockfd,5); // espera bloqueado por una respuesta 
	
		clilen = sizeof(cli_addr);

		newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
		if(newsockfd < 0){
			errco = 1;
			return NULL;
		}
		errco = 0;
	
		sem_wait(&tablero);//blokea semaforo
    	sem_post(&tablero);


		n = write(newsockfd,&paquete,sizeof( struct package));
		if(n < 0){
			errco = 3;
			return NULL;
		}

		if (jugador != 0) {n = read(newsockfd,&paquete.jugadores[0].keyst,sizeof(char));
            if(n < 0){
			errco = 4;
			return NULL;
		}
		control(0);
        };

	} //fin del while
	close(newsockfd);
	close(sockfd);
	
	return NULL;

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
		case 1: if (paquete.col_maze[paquete.jugadores[i].y+1][paquete.jugadores[i].x] == 0) {return 0;} else if (paquete.col_maze[paquete.jugadores[i].y+1][paquete.jugadores[i].x] == 1 && i != 0) {return 0;} else return 1; break;
		case 2: if (paquete.col_maze[paquete.jugadores[i].y-1][paquete.jugadores[i].x] == 0) {return 0;} else if (paquete.col_maze[paquete.jugadores[i].y-1][paquete.jugadores[i].x] == 1 && i != 0) {return 0;} else return 1; break;
		case 3: if (paquete.col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x+1] == 0) {return 0;} else if (paquete.col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x+1] == 1 && i != 0) {return 0;} else return 1; break;
		case 4: if (paquete.col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x-1] == 0) {return 0;} else if (paquete.col_maze[paquete.jugadores[i].y][paquete.jugadores[i].x-1] == 1 && i != 0) {return 0;} else return 1; break;
		default: return 1;
	}
}

void init_laby(){
	int x, y;
	
	for (y = 0; y <= LABY_HEIGHT-1 ; y++){
		for (x = 0; x <= LABY_WIDTH-2; x++){
		
			switch (maze[y][x]){
				case ' ': paquete.col_maze[y][x] = 0; paquete.cur_maze[y][x] = 0; paquete.pills++; break;
				case 'p': paquete.col_maze[y][x] = 0; paquete.cur_maze[y][x] = 1; break;
                case 'o': paquete.col_maze[y][x] = 0; paquete.cur_maze[y][x] = -1; xo = x; yo = y; break;
                case 'i': paquete.col_maze[y][x] = 0; paquete.cur_maze[y][x] = -1; xi = x; yi = y; break;
                case 'y': paquete.col_maze[y][x] = 0; paquete.cur_maze[y][x] = -1; break;
                case 'w': paquete.col_maze[y][x] = 0; paquete.cur_maze[y][x] = -1; break;
                case 'f': paquete.col_maze[y][x] = 1; paquete.cur_maze[y][x] = -1; break;
				default:  paquete.col_maze[y][x] = -1; paquete.cur_maze[y][x] = -1;
			}
		}
	}
	
}

void control (char i){

    static int dir[5];
    
if (paquete.jugadores[i].death != 1) {
	switch (paquete.jugadores[i].keyst){
      case('A'): if (!collition(i,dir[i] = 2)) {paquete.jugadores[i].ax = 0; paquete.jugadores[i].ay = -1;} break;
      case('B'): if (!collition(i,dir[i] = 1)) {paquete.jugadores[i].ax = 0; paquete.jugadores[i].ay = 1; } break;
      case('C'): if (!collition(i,dir[i] = 3)) {paquete.jugadores[i].ax = 1; paquete.jugadores[i].ay = 0; } break;
      case('D'): if (!collition(i,dir[i] = 4)) {paquete.jugadores[i].ax = -1; paquete.jugadores[i].ay = 0;} break;
      case
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
    
    while (paquete.game_st == 0) {
        
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
    
    if (paquete.jugadores[0].death || (paquete.pu == 2 && jugador != 0)) {mvwprintw(screen, y_in_laby(paquete.jugadores[0].y), x_in_laby(screen, paquete.jugadores[0].x), " " );}
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
		if (!paquete.pp && paquete.jugadores[y].death == 0) { wattron(screen, COLOR_PAIR(y+3)); mvwprintw(screen, y_in_laby(paquete.jugadores[y].y), x_in_laby(screen, paquete.jugadores[y].x), "ᗝ" ); 
            
            wattron(screen, COLOR_PAIR(1));}
		
		else if (paquete.pp && paquete.jugadores[y].death == 0) { wattron(screen, COLOR_PAIR(1)); mvwprintw(screen, y_in_laby(paquete.jugadores[y].y), x_in_laby(screen, paquete.jugadores[y].x), "ᗝ" );}
		
        else if (paquete.jugadores[y].death == 1) {mvwprintw(screen, y_in_laby(paquete.jugadores[y].y), x_in_laby(screen, paquete.jugadores[y].x), " " );}    
	}
}

void collition_pac (WINDOW *screen) {
    static int pp_start;
    static unsigned long end;
    int y;
    
    if (paquete.cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] == 0) {paquete.cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] = -2; paquete.pills--; if (pu_c < 30) {paquete.pu_c++;}; };
    if (paquete.cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] == 1) {paquete.cur_maze[paquete.jugadores[0].y][paquete.jugadores[0].x] = -2; paquete.pp = 1; end = paquete.msec/1000 + 20;};
    if (end == paquete.msec/1000) {paquete.pp = 0;};
    mvwprintw(screen, 16, 2, "%d", end);
    
    for (y = 1; y < 5; y++){
        if ( (paquete.jugadores[0].x == paquete.jugadores[y].x && paquete.jugadores[0].y == paquete.jugadores[y].y) && paquete.pp == 0) {paquete.jugadores[0].death = 1;}
        if ( (paquete.jugadores[0].x == paquete.jugadores[y].x && paquete.jugadores[0].y == paquete.jugadores[y].y) && paquete.pp == 1) {paquete.jugadores[y].death = 1;}
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
  if (paquete.jugadores[i].death == 1 && end[i] == 0) { paquete.jugadores[0].lives--; end[i] = paquete.msec/1000 + 20; restore_player(i);}
  if (paquete.jugadores[i].death == 1 && end[i] == paquete.msec/1000) {paquete.jugadores[i].death = 0; end[i] = 0;}
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
			
			switch (paquete.cur_maze[y][x]) {
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

signed char menu_cursor (char i, char y, char z) {
    switch (y) {
      case('A'): if (i == 0) {return z;} else {return i-1;};
      case('B'): if (i == z) {return 0;} else {return i+1;};
      default: return i;
    }
}

signed char menu (WINDOW *screen) {
    
    signed char cursor = 0;
    char w_cursor = ' ';
    
    do {
        
        w_cursor = ' ';
        
        wclear(screen);
    mvwprintw (screen, y_in_laby(0), x_in_laby(screen, 5), "Pᗧc Mᗣn");
    
    
     mvwprintw (screen, y_in_laby(2), x_in_laby(screen, 3), "Iniciar Partida");
     mvwprintw (screen, y_in_laby(4), x_in_laby(screen, 3), "Unirse a Partida");
     mvwprintw (screen, y_in_laby(6), x_in_laby(screen, 3), "Instrucciones");
     mvwprintw (screen, y_in_laby(8), x_in_laby(screen, 3), "Salir");
     
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
     cursor = menu_cursor(cursor, w_cursor, 3);
     if (w_cursor == 'q') {
         exit(0);
    }
     
     
     
    wrefresh(screen);
    
    } while (1);
return cursor;
}

int sel_menu (WINDOW *screen) {
    
    signed char cursor = 0;
    char w_cursor = ' ';
    
    do {
        
        w_cursor = ' ';
        
        wclear(screen);
    mvwprintw (screen, y_in_laby(0), x_in_laby(screen, 5), "Seleccione un personaje");
    
    wattron(screen, COLOR_PAIR(2));
    mvwprintw (screen, y_in_laby(2), x_in_laby(screen, 3), "ᗧ");
    
    wattron(screen, COLOR_PAIR(4));
     
    mvwprintw (screen, y_in_laby(4), x_in_laby(screen, 3), "ᗝ");
    
    wattron(screen, COLOR_PAIR(5));
     
    mvwprintw (screen, y_in_laby(6), x_in_laby(screen, 3), "ᗝ");
    
    wattron(screen, COLOR_PAIR(6));
     
    mvwprintw (screen, y_in_laby(8), x_in_laby(screen, 3), "ᗝ");
    
    wattron(screen, COLOR_PAIR(7));
    
    mvwprintw (screen, y_in_laby(10), x_in_laby(screen, 3), "ᗝ");
    
    
    wattron(screen, COLOR_PAIR(1));
    
    mvwprintw (screen, y_in_laby(12), x_in_laby(screen, 3), "Volver");
     
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
         case 4:
             mvwprintw (screen, y_in_laby(10), x_in_laby(screen, 0), "➤");
             break;
         case 5:
             mvwprintw (screen, y_in_laby(12), x_in_laby(screen, 0), "➤");
             break;
    }
    
    if (cursor != 5) {jugador = cursor;}
     w_cursor = wgetch(screen);
     
     if (w_cursor == 'q') {
         exit(0);
    }
    
     if (w_cursor == '\r') {
        if (cursor == 5) {jugador = -1; break;}
        else {
            if (server == 1) {break;}
            else if (server == 0) { 
                    if (client() == 2) {mvwprintw (screen, y_in_laby(14), x_in_laby(screen, 0), "Personaje ya seleccionado o error de conexion");}
                    else {break;}
            }
        }
    }
     cursor = menu_cursor(cursor, w_cursor, 5);
     
     
    wrefresh(screen);
    
    } while (w_cursor != '\r');
}



void err_display (WINDOW *field) {
    switch (errco) {
         case 0: break;
         case 1: mvwprintw(field, 3, 4, "Error de Conexion"); break;
         case 2: mvwprintw(field, 3, 4, "Error de apertura de puerto"); break;
         case 3: mvwprintw(field, 3, 4, "Error de envio"); break;
    }
}



int game_loop (WINDOW *w, WINDOW *field, WINDOW *score) {
    
    
  noraw();
  cbreak();
  nodelay(stdscr,TRUE);
  nodelay(w, TRUE);
  nodelay(field, TRUE);
  nodelay(score, TRUE);
  timeout(0);
  wtimeout(field, 0);
  
      pthread_create(&mov0, NULL, movimiento, (void*)0);
      pthread_create(&mov1, NULL, movimiento, (void*)1);
      pthread_create(&mov2, NULL, movimiento, (void*)2);
      pthread_create(&mov3, NULL, movimiento, (void*)3);
      pthread_create(&mov4, NULL, movimiento, (void*)4);
  
  
  init_laby();
  
  
  draw_borders(score);
  
    wrefresh(score);
    mvwprintw(score, 1, 1, "Puntuacion");
    
if (server == 1) {start = clock();}
    
while (paquete.game_st == 0) {

    death_manager(field);
	resize(parent_y, parent_x, field, score);
    
    err_display(field);

    key = wgetch(field);

	usleep(DELAY);
	
	if (server == 1) {
        diff = clock() - start;
        paquete.msec = (diff + DELAY) * 1000 / CLOCKS_PER_SEC;
    }
	
	
	

    
    wclear(stdscr);
     wclear(field);
     wclear(score);
    // draw to our windows
	draw_laby(field, score);

	wattron(score, COLOR_PAIR(2)); mvwprintw(score, 2, 5, "%d",paquete.jugadores[0].lives); wattron(score, COLOR_PAIR(1));

	draw_borders(score);
	mvwprintw (score, 1, 2, "%d", paquete.msec/1000);
    
    
    if (server == 1) {
        if (key != ERR) paquete.jugadores[jugador].keyst = key;
         control(jugador);
    }
    if (server == 0) {
        if (key != ERR) keyst_local = key; 
        client();
    }
    
    if (paquete.game_st == 1) return -1;
    if (paquete.pills == 0) return 0;
    if (paquete.jugadores[0].lives == 0) return 1;
    if (paquete.jugadores[jugador].keyst == 'q' || keyst_local == 'q') return 2;
    
    if (server != 0) {collition_pac (field);} 
    
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

void init_server () {
    static int init = 0;
    
    //if (paquete.game_st == 1) {init = 0;}
    
   if (init == 0) {
   sem_init(&tablero, 0, 1);
   pthread_create(&serv1, NULL , server_fantasma , (void*)1);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv2, NULL , server_fantasma , (void*)2);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv3, NULL , server_fantasma , (void*)3);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv4, NULL , server_fantasma , (void*)4);//llamada a la funcion que ejecuta el hilo
   pthread_create(&serv5, NULL , server_pacman , NULL);
   init++;
   }
}




int main(int argc, char *argv[]) {
 
 ip = "127.0.0.1";

setlocale(LC_ALL, "");
  
    
  init_ncurses();

  getmaxyx(stdscr, parent_y, parent_x);
  
  WINDOW *w = initscr();;
  WINDOW *field = newwin(parent_y - SCORE_SIZE, parent_x, 0, 0);
  WINDOW *score = newwin(SCORE_SIZE, parent_x, parent_y - SCORE_SIZE, 0);
  
  
while (1) {
jugador = -1;
init_players();

while (jugador == -1){
  switch (menu(field)) {
      case 0: server = 1; sel_menu(field); init_server(); paquete.game_st = 0; break;
      case 1: server = 0; sel_menu(field); break;
      case 2: break;
      case 3: paquete.game_st = 1; wclear(score); wclear(field); wclear(w); endwin(); exit(0); return 0; break;
}
}



switch (game_loop(w, field, score)) {
      case 0:
          mvwprintw(field, 4, 4, "Pacman a Ganado");
          mvwprintw(field, 5, 4, "Gracias por jugar");
          paquete.game_st = 1;
          break;
      case 1:
          mvwprintw(field, 4, 4, "Pacman a Perdido");
          mvwprintw(field, 5, 4, "Gracias por jugar");
          paquete.game_st = 1;
          break;
      case 2:
          mvwprintw(field, 5, 4, "Has salido de la partida");
          paquete.game_st = 1;
          break;
      case -1:
          mvwprintw(field, 5, 4, "La partida a sido cerrada");
          paquete.game_st = 1;
          break;
}
wclear(score);
wrefresh(field);
sleep(2);
}

  endwin();	
  
  exit(0);

  return 0;
}


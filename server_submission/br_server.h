#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>


#define BUFFER_SIZE 1024
#define NO_OF_CLIENTS 3
#define NO_OF_LIVES 5




// If true, we should tear down the surver
extern bool tear_server;


// slows down the game play so you can test for clients
// disconnecting or people trying to join mid game
extern bool debug;

// For the time out function
extern struct timeval timeout;
extern fd_set set;



/**
 * Struct which stores the current game state
 * When there is only a 1 player implementation, it can be useful to
 * also store the number of rounds to see how far it gets.
 */
typedef struct state{
    int life[NO_OF_CLIENTS]; // life of player (starts at 3).
    int client_id[NO_OF_CLIENTS];
} STATE;

/**
 * Enumerated type for all the possible moves that could be
 * made by the client.
 */
typedef enum {Eve,// even
    Odd, // odd
    Dou, // double
    One, // contains 1
    Two, // contains 2
    Thr, // contains 3
    Fou, // contains 4
    Fiv, // contains 5
    Six, // contains 6
    Inv  // invalid move
} MOVE;

extern MOVE move;


// ----------------------------------------------------------------------

extern bool check_start(int *clients, int num_players, STATE *s);

extern void check_connection(int server_fd);

extern bool end_of_game(STATE *s, int *client);

extern void check_elimination(STATE *s, int *client);

extern void play_round(int * dice, int client, STATE *s, int cli_no);

extern int setup_clients(int server_fd ,int * client_fd, STATE *s);

extern bool init_client(int client, int client_id);

extern int gen_client_id();

extern int setup_server(int port);

extern void send_no_id_to_client(int client, char *mess);

extern void send_start_to_client(int client, int num_player);

extern void send_welcome_to_client(int client, int client_id);

extern void send_to_client(int client, int client_id, char *mess);

extern char *Responder(int * dice, MOVE m , STATE *s, int cli_no);

extern void tear(int server_fd);

extern int *rollDice(void);

extern MOVE client_responce(char* buf);



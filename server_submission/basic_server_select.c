/**
* Based on code found at https://github.com/mafintosh/echo-servers.c (Copyright (c) 2014 Mathias Buus)
* Copyright 2019 Nicholas Pritchard, Ryan Bunney
**/

/**
 * @brief An implementation of a TCP network that utilises the select function
 * to iterate over all the clients and evaluate the state of the game
 */

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
bool tear_server = false;

// For the time out function
struct timeval timeout;
fd_set set;

// slows down the game play so you can test for clients
// disconnecting or people trying to join mid game
bool debug = true;

/**
 * Enumerated type for all the possible moves that could be
 * made by the client.
*/
typedef enum valid_move {Eve,// even 
                        Odd, // odd
                        Dou, // double
                        One, // contains 1
                        Two, // contains 2
                        Thr, // contains 3
                        Fou, // contains 4
                        Fiv, // contains 5
                        Six, // contains 6
                        Inv  // invalid move
                        } move;


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
 * Funciton that takes a message from a client and 
 * translates it into an enumerated type - move.
 * 
 * The function will pick up on the first instance of one of the valid 
 * moves, our interpretation of how to gracefully interprate packets.
 * 
 * 
 * @param: string from the client
 * 
 * @return: enumerated type of the move they have sent
 */
move client_responce(char* buf){

    if (strstr(buf, "EVEN") != NULL) {
        return Eve;
    }else 
    if (strstr(buf, "ODD") != NULL) {
        return Odd;
    } else 
    if(strstr(buf, "DOUB") != NULL){
        return Dou;
    }else
    if(strstr(buf, "CON,1") != NULL){
        return One;
    }else 
    if(strstr(buf, "CON,2") != NULL){
        return Two;
    }else 
    if(strstr(buf, "CON,3") != NULL){
        return Thr;
    }else 
    if(strstr(buf, "CON,4") != NULL){
        return Fou;
    }else 
    if(strstr(buf, "CON,5") != NULL){
        return Fiv;
    }else 
    if(strstr(buf, "CON,6") != NULL){
        return Six;
    } else{
        // Can not recognise the message being sent
        // Interpreting this as different to cheating as 
        // the player has not attempted to make a move
        return Inv;
    }
}


/**
 * Roll dice function returns an int array of the values of both die.
 * 
 * @return: a 1x2 integer array of numbers between 1 & 6
 */
int *rollDice(void){
    static int result[2];
    result[0] = rand() % 6 + 1;
    result[1] = rand() % 6 + 1;

    // for testing
    printf("first dice: %d, second dice: %d \n", result[0], result[1]);

    return result;
}


/**
 * Function which (gracefuly) tears down the server.
 * This should not be called until all clients have been sent a
 * message to shut themselves down.
 * 
 * @param: the server file descriptor
 */
void tear(int server_fd){
    close(server_fd);
    printf("server shutdown\n");
    exit(EXIT_SUCCESS);
}

 
/**
 * Function receives dice roll result and a valid client guess.
 * It then decides which course of action to take
 * 
 * @param dice: a 1x2 integer array of the dice roll for this round
 * @param m: an enumerated valid move as interpreted by client responce
 * @param s: the game state holding the clients ID and the number of lives
 * @param cli_no: integer of the client we are evaluating (for readability)
 * 
 * @return: the string that should be sent to the client
 */
char *Responder(int * dice, enum valid_move m , STATE *s, int cli_no){
    // Appropriate response for client (always 4 characters)
    char *response = malloc(4);
    switch(m){
        case Eve :
            if((dice[0] + dice[1])%2 == 0){
                printf("correct guess, dice results are even\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice roll is odd\n");
                s->life[cli_no]--;
                response = "FAIL";
            }
            break;

        case Odd :
            if((dice[0] + dice[1])%2 != 0){
                printf("correct guess, dice results are odd\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice roll is even\n");
                s->life[cli_no]--;
                response = "FAIL";
            }
            break;

        case Dou :
            if(dice[0] == dice[1]){
                printf("correct guess, dice results are doubles\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice roll isnt doubles\n");
                s->life[cli_no]--;
                response = "FAIL";
            }
            break;

        case One :
            if(dice[0] == 1 || dice[1] == 1){
                printf("correct guess, a dice is 1\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice is not 1\n");
                s->life[cli_no]--;
                response = "FAIL";
            }            
            break;

        case Two :
            if(dice[0] == 2 || dice[1] == 2){
                printf("correct guess, a dice is 2\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice is not 2\n");
                s->life[cli_no]--;
                response = "FAIL";
            }            
            break;

        case Thr :
            if(dice[0] == 3 || dice[1] == 3){
                printf("correct guess, a dice is 3\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice is not 3\n");
                s->life[cli_no]--;
                response = "FAIL";
            }             
            break;

        case Fou :
            if(dice[0] == 4 || dice[1] == 4){
                printf("correct guess, a dice is 4\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice is not 4\n");
                s->life[cli_no]--;
                response = "FAIL";
            }
            break;

        case Fiv :
            if(dice[0] == 5 || dice[1] == 5){
                printf("correct guess, a dice is 5\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice is not 5\n");
                s->life[cli_no]--;
                response = "FAIL";
            }             
            break;

        case Six :
            if(dice[0] == 6 || dice[1] == 6){
                printf("correct guess, a dice is 6\n");
                response = "PASS";
            }else{
                printf("incorrect guess, dice is not 6\n");
                s->life[cli_no]--;
                response = "FAIL";
            }             
            break;
        case Inv :
                printf("could not understand move\n");
                s->life[cli_no]--;
                response = "FAIL";
                       
            break;
    }
    return response;
}


/**
 * Helper function that sends a move message to the client.
 * Makes sure the buffer is cleared before anything is sent to the client.
 * 
 * @param client: the client file descriptor
 * @param client_id: the id of the client
 * @param mess: string that will be sent to the client
 */
void send_to_client(int client, int client_id, char *mess){
    char *buf;
    int err;
    buf = calloc(BUFFER_SIZE, sizeof(char)); 
    buf[0] = '\0';

    printf("server sends %s to client %d\n", mess, client_id);
    sprintf(buf,"%d,%s", client_id, mess);
    err = send(client, buf, strlen(buf), 0); // Send our response
    if (err < 0){
        fprintf(stderr,"Client send failed\n");
        exit(EXIT_FAILURE);
    }

    // Free this buffer since we will make a new one for the next client
    free(buf);
}

/**
 * Helper function that sends a WELCOME message to the client.
 * needs to be a separate function since its WELCOME,001 not 001,WELCOME.
 * 
 * @param client: client file descriptor
 * @param client_id: the client id
 * (don't need a message parameter) 
 */
void send_welcome_to_client(int client, int client_id){
    char *buf;
    int err;
    buf = calloc(BUFFER_SIZE, sizeof(char)); 
    buf[0] = '\0';

    printf("server sends WELCOME to client %d\n", client_id);
    sprintf(buf,"WELCOME,%d", client_id);
    err = send(client, buf, strlen(buf), 0); 
    if (err < 0){
        fprintf(stderr,"Client send failed\n");
        exit(EXIT_FAILURE);
    }
    free(buf);
}


/**
 * Helper function that sends the start message to the client
 * needs to be a separate function since it needs the number of
 * clients as well as their ids.
 * 
 * @param client: client file descriptor
 * @param num_player: the number of players in the game
 * @param lives: how many lives the player will have
 */
void send_start_to_client(int client, int num_player){
    char *buf;
    int err;
    buf = calloc(BUFFER_SIZE, sizeof(char)); 
    buf[0] = '\0';

    // Send 2 digit numbers so check whether we have a 1 or 2 digit number

    // For our current implementation, it only plays with 4 players. To allow for more, 
    // add another if condition here to send the right number of integers.
    if(NO_OF_LIVES>9){
        sprintf(buf,"START,0%d,%d", num_player, NO_OF_LIVES);
        err = send(client, buf, strlen(buf), 0); 
    } 
    else{
        sprintf(buf,"START,0%d,0%d", num_player, NO_OF_LIVES);
        err = send(client, buf, strlen(buf), 0);    
    }

    if (err < 0){
        fprintf(stderr,"Client send failed\n");
        exit(EXIT_FAILURE);
    }
    free(buf);
}

/**
 * Helper function that sends a message without an ID
 * e.g. REJECT and CANCEL
 * 
 * @param client: client file descritor
 * @param mess: the message you want to send to the client
 */
void send_no_id_to_client(int client, char *mess){
    char *buf;
    int err;
    buf = calloc(BUFFER_SIZE, sizeof(char)); 
    buf[0] = '\0';

    sprintf(buf,"%s", mess);
    err = send(client, buf, strlen(buf), 0); 
    if (err < 0){
        fprintf(stderr,"Client send failed\n");
        exit(EXIT_FAILURE);
    }
    free(buf);
}


/**
 * Sets up the game server given a port number.
 * 
 * @param port: the command line argument of which port to bind to
 * 
 * @return: the socket file descriptor
 */
int setup_server(int port){

    int server_fd, err, opt_val;
    struct sockaddr_in server;

    // Zero out the memory for the server information
    memset(&server, 0, sizeof(server));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    // Setup surver info
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err < 0){
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    } 

    return server_fd;
}

/**
 * Return a random 3 digit number to give to the client.
 * There is a very small possibility that two clients could be assigned
 * the same number. This will not effect the server as they will
 * still have different indicies in the game state.
 * 
 * @return: a random 3 digit number
 */
int gen_client_id(){
    return rand() % 900 + 100;
}

/**
 * Initialise the clients and check if they have sent an INIT.
 * If they have sent an INIT, send a welcome in return,
 * otherwise, send them a REJECT message
 * 
 * @param client: the client file descriptor
 * @param client_id: the randomly generated id
 */
bool init_client(int client, int client_id){
    int err;
    char *buf;
    buf = calloc(BUFFER_SIZE, sizeof(char)); // Clear our buffer so we don't accidentally send/print garbage
    
    // Try to receiving INIT from client
    int read = recv(client, buf, BUFFER_SIZE, 0);

    if (read < 0){
        // Couldnt read
        fprintf(stderr,"Client read failed\n");
        exit(EXIT_FAILURE);
    }

    // Check that the client sent an INIT call
    if (strstr(buf, "INIT") != NULL) {
        // If we can connect, send a welcome to the client
        send_welcome_to_client(client, client_id);
        return true;
    } 
    else {
        // If the client didn't send an INIT
        send_no_id_to_client(client, "REJECT");
        return false;
    }
    free(buf);
}


/**
 * Sets up the client connection and calls init client if the 
 * setup was successful
 * 
 * @param server_fd: the server file descriptor
 * @param client_fd: the client file descriptor (usually just called client)
 * @param s: the state of the game
 * 
 * @return: the number of connections to the server
 */
int setup_clients(int server_fd ,int * client_fd, STATE *s){
    struct timeval stop, start;
    int err;
    struct sockaddr_in client;
    int nready;
    int num_conn = 0;

    //start timing how long until clients join game
    gettimeofday(&start, NULL);

    while(num_conn< NO_OF_CLIENTS){

        // Listen for clients
        err = listen(server_fd, 128);
        if (err < 0){
            fprintf(stderr,"Could not listen on socket\n");
            exit(EXIT_FAILURE);
        } 

        // Zero out memory for the client information
        memset(&client, 0, sizeof(client));
        socklen_t client_len = sizeof(client);

        gettimeofday(&stop, NULL);
        // Calculate how much time has elapsed since last select function
        timeout.tv_sec = 30 - (stop.tv_sec - start.tv_sec);
        // For testing how long the server is waiting for clients
        printf("time left to join: %ld\n", timeout.tv_sec);
        FD_ZERO(&set);
        FD_SET(server_fd, &set);
        select(server_fd + 1, &set, NULL, NULL, &timeout);
        

        // Server times out after allowing 30 seconds for clients to join. 
        // If no clients join function returns 0. Otherwise returns #clients
        nready = FD_ISSET(server_fd, &set);

        // If there's a time out:    
        if(nready == 0){
            // Returns number of connections if a time out occurs.
            return num_conn; 
        }

        // Accept the connection from the client
        client_fd[num_conn] = accept(server_fd, (struct sockaddr *) &client, &client_len);
        if (client_fd[num_conn] < 0) {
            fprintf(stderr,"Could not establish new connection\n");
            exit(EXIT_FAILURE);
        } 

        // Assign the client an ID
        s->client_id[num_conn] = gen_client_id();

        // If the client was corrently initialised, increase client count
        if(init_client(client_fd[num_conn], s->client_id[num_conn])){
            printf("Accepted connection from client %d\n", num_conn);
            num_conn++;
        }else{
            printf("Client %d was not initialised\n", s->client_id[num_conn]);
        }
    }
    return num_conn;
}


/**
 * Function that plays a round of the game for only 1 player.
 * Handles player time out if they dont make a move within 3 seconds
 * Sends game state to the responder function to tell the server how to respond.
 * 
 * If the player has left the game, it will be caught here. The number of lives
 * will be set to -2 so the sever doesnt try to send an elination to them.
 * 
 * If the client sends more than a 14 character response, this is interpreted
 * as cheating (e.g. client sends 001,MOV,EVEN,ODD,DOUB).
 * 
 * @param dice: a 1x2 array of the dice roll. This needs to be passed to
 *              the responder function.
 * @param client: the client file descriptor
 * @param s: the state of the game
 * @param cli_no: the index of the client we should use in state
 */
void play_round(int * dice, int client, STATE *s, int cli_no){
    char *buf;
    buf = calloc(BUFFER_SIZE, sizeof(char)); 

    // Player has a limit of 3 seconds to make their move
    timeout.tv_sec = 3;
    FD_ZERO(&set);
    FD_SET(client, &set);
    select(client+1, &set, NULL, NULL, &timeout);

    int nready = FD_ISSET(client, &set);

    // the client has timed out
    if(nready == 0){
        s->life[cli_no]--;
        printf("Timeout for clinet %d\n", s->client_id[cli_no]);
        send_to_client(client,s->client_id[cli_no], "FAIL");
    }else{
        // Read message from the client
        int read = recv(client , buf, BUFFER_SIZE, O_NONBLOCK);

        // If read is less than or equal to 0 if client has dissconnected
        if (read <= 0){
            printf("client %d has left the game\n", s->client_id[cli_no]);
            s->life[cli_no] = -2;


        // Player cheating: send elimination 
        }else if(strlen(buf) > 14){
            printf("Shame on you client %d, you just got caught cheating!\n", s->client_id[cli_no]);
            s->life[cli_no] = -1;
        }else{
            // Relay what client said and call interpreter
            printf("client sent %s\n", buf);
            move m = client_responce(buf);

            // For testing
            if(debug){
                sleep(1);
            }

            // Send our response to the client
            send_to_client(client, s->client_id[cli_no], Responder(dice, m, s, cli_no));
        }
    }
    free(buf);
}


/**
 * Called at the end of the round and checks to see if a client 
 * needs to be eliminated or if someone has won.
 * 
 * Modifies global variable tear server to be true of false
 * 
 * There are 4 options here:
 *      1) There is a draw and we need to notify the players who have won
 *      2) Once client has lost and needs to be eliminated
 *      3) We have a single winner and we need to tell them
 *      4) all players cheat/leave the game and have been eliminated and
 *         no one wins
 * 
 * @param s: the state of the game
 * @param client: an array of all the client file descritors to
 *                itterate through 
 */
void check_elimination(STATE *s, int *client){

    // Have a counter to check the case of last two players
    int clients_playing = 0;
    int emilinated = 0;
    bool draw = true;

    for(int i =0; i < NO_OF_CLIENTS; i++){
        // If there is more than 1 person with life > 1
        // Then we dont have a draw and increase the number of clients playing
        if(s->life[i] > 0){
            clients_playing++;
            draw = false;
        }
    }

    // If its a draw, itterate through and tell clients they have won, 
    // Then tear down the surver
    if(draw == true){
        for(int i =0; i < NO_OF_CLIENTS; i++){
            if(s->life[i] == 0){
                send_to_client(client[i],s->client_id[i], "VICT");
                printf("there was a draw\n");
            }
        }
        tear_server = true;
        return;
    }

    // Itterate through life array to see if anyone is 0
    // If they are, send eliminations 
    //(draw cases should have been delt with already)
    for(int i =0; i < NO_OF_CLIENTS; i++){

        // If they have no lives but they havent been eliminated
        // Eliminate them
        if(s->life[i] == 0){
            // Send elimination
            printf("eliminating client\n");
            send_to_client(client[i], s->client_id[i], "ELIM");
            // mark that they have been eliminated
            s->life[i] = -1; 
        }
    }

    // If there is only 1 player remaining, they should win
    if(clients_playing == 1){
        printf("one client playing\n");
        // itterate through to find remaining client
        for(int i =0; i < NO_OF_CLIENTS; i++){
            if(s->life[i] > 0){
                send_to_client(client[i],s->client_id[i], "VICT");
                tear_server = true;
                return;
            }
        }
    }

    // Prevent infinite loop or when all players try to 
    // cheat at the same time or all leave
    if(clients_playing==0){
        printf("no players left, error has occured\n");
        tear_server = true;
        return;
    }
}


/**
 * Prints out all the end of round information and calls 
 * check_eliniation to see whether anyone has won or if it's 
 * the end of the game.
 * 
 * @return: true if the server needs to be torn down, false otherwise
 */ 
bool end_of_game(STATE *s, int *client){

    if(debug){
        printf("---  end of round  ---\n");
        printf("| player | lives left|\n");
        for(int i=0; i < NO_OF_CLIENTS; i++){
            printf("|   %d  |    %d      |\n", s->client_id[i], s->life[i]);
        }
        printf("-----------------------\n");
    }

    // check for eliminations
    check_elimination(s, client);

    if(tear_server){
        return true;
    }else{
        return false;
    }
}

/**
 * Helper function which checks whether a new player has tried to join the game
 * Called at the end of each round.
 * 
 * The server is changed to NON_BLOCKING once the game has started, making sure
 * it doesnt wait to see if there is a conenction at the end of each round. It
 * only checks to see if theres anyone there, then moves on.
 * 
 * The client connection must be accepted before a reject can be sent.
 * 
 * @param server_fd: the (NON_BLOCKING) server file descritor
 */
void check_connection(int server_fd){
    int err;
    int flags;
    // Will have to make a new client in order to send the REJECT message
    int client_fd;
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    err = listen(server_fd, 128);
    if (err < 0){
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    }

    // Accept client    
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
    if (client_fd < 0) {
        return;
    } 

    send_no_id_to_client(client_fd,"REJECT");
}

/**
 * Check function to see if we have enough players to start a game
 * If we do, send START and initialise the life array
 * 
 * @return: true if there wasnt enough players to start, 
 *          false if we CAN start the game
 */
bool check_start(int *clients, int num_players, STATE *s){
    //check if enough players joined and if not shut down server
    if(num_players < NO_OF_CLIENTS){
        printf("%d is not enough players to have a game\n", num_players);
        for(int i = 0; i < num_players; i++){
            send_no_id_to_client(clients[i], "CANCEL");
        }
        return true;
    }
    else{
        // We have enough players, send start to the clients,
        // Give them all the number of lives
        for(int i = 0; i < num_players; i++){
            send_start_to_client(clients[i], num_players);
            s->life[i] = NO_OF_LIVES;
        } 
        return false;
    }
}


/**
 * Main function which implements the server.
 * The steps it performs are:
 *      1) Sets up the server according to the given port number
 *      2) Sets up all the clients that connect, if there aren't enough
 *         connections the server is torn down
 *      3) Play rounds of the game until we have a winner
 *      4) Tear down the server
 */
int main (int argc, char *argv[]) {
    
    // Initialise state variables for this game
    STATE s;
    int server_fd, client_fd, err;

    // Check if we have enough command line arguments
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    // Call function to set up the server
    server_fd = setup_server(port);
        
    // Initialise clients
    int *clients = (int*)malloc(sizeof(int) * NO_OF_CLIENTS);
    memset(clients, 0, NO_OF_CLIENTS*sizeof(int));
    int num_players =  setup_clients(server_fd, clients, &s);


    // Check if we have enough players to start the game
    if(check_start(clients, num_players, &s)){
        tear(server_fd);
    }else{
        // Set the server to be non blocking, 
        // i.e. it doesn't wait for incomming connections
        int non_blocking = fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL, 0) | O_NONBLOCK);
    }
    
    // Play the game
    while (true) {  
        if(debug){printf("------ new round! -----\n");}

        // Roll dice and save variable 
        int * dice = rollDice();

        // Iterate over all the players for the round
        // This can be improved later using threading?
        for(int i=0; i < NO_OF_CLIENTS; i++){
            if(s.life[i] > 0){
                if(debug){printf("---  Player %d    ---\n", s.client_id[i]);}
                play_round(dice, clients[i], &s, i);
            }else{
                // Skip eliminated clients
                continue;
            }
        }

        //reject any players that may be listening
        check_connection(server_fd);

        // Commence end of round checks:
        if(end_of_game(&s, clients)){
            tear(server_fd);
        }
    }
}

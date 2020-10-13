/**
* Based on code found at https://github.com/mafintosh/echo-servers.c (Copyright (c) 2014 Mathias Buus)
* Copyright 2019 Nicholas Pritchard, Ryan Bunney
**/

/**
 * @brief A simple example of a network server written in C implementing a basic echo
 * 
 * This is a good starting point for observing C-based network code but is by no means complete.
 * We encourage you to use this as a starting point for your project if you're not sure where to start.
 * Food for thought:
 *   - Can we wrap the action of sending ALL of out data and receiving ALL of the data?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#define BUFFER_SIZE 1024


/**
 * Enumerated type for all the possible moves that could be
 * made by the client
*/
typedef enum valid_move {Eve,// even 
                        Odd, // odd
                        Dou, // double
                        One, // 1
                        Two, // 2
                        Thr, // 3
                        Fou, // 4
                        Fiv, // 5
                        Six  // 6
                        } move;


/**
 * Struct which stores the current game state
*/
typedef struct state{
    int round; // which round the game is on
    int life; // life of player (starts at 3).
    int Number_of_Players;
} STATE;




int client_id;


/**
 * Funciton that takes a message from a client and translates it into an enumerated type - move
*/
move client_responce(char* buf){

    if (strstr(buf, "EVEN") != NULL) {  // Check if the message contained 'EVEN'
        printf("message contains EVEN\n");
        return Eve;
    }else 
    if (strstr(buf, "ODD") != NULL) {  // Check if the message contained 'ODD'
        printf("message contains ODD\n");
        return Odd;
    } else 
    if(strstr(buf, "DOUB") != NULL){
        printf("message contains DOUB\n");
        return Dou;
    }else
    if(strstr(buf, "CON,1") != NULL){
        printf("message contains CON,1\n");
        return One;
    }else 
    if(strstr(buf, "CON,2") != NULL){
        printf("message contains CON,2\n");
        return Two;
    }else 
    if(strstr(buf, "CON,3") != NULL){
        printf("message contains CON,3\n");
        return Thr;
    }else 
    if(strstr(buf, "CON,4") != NULL){
        printf("message contains CON,4\n");
        return Fou;
    }else 
    if(strstr(buf, "CON,5") != NULL){
        printf("message contains CON,5\n");
        return Fiv;
    }else 
    if(strstr(buf, "CON,6") != NULL){
        printf("message contains CON,6\n");
        return Six;
    } else{
        // do not understand message sent:
        fprintf(stderr, "Unexpected message, terminating\n");
        exit(EXIT_FAILURE);
    }
}


/**
 * Roll dice function returns an int array of the values of both die
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
 * Function which tears down the server (gracefully?)
*/
void tear(int server_fd){
    close(server_fd);
    printf("server shutdown\n");
    exit(0);
}


// If true, we should tear down the surver
bool tear_server = false;


/**
 * function to be called to check whether the client has won or not
*/
char *check_win(STATE *s){
    char *response = malloc(4);
    //We have completed the final round
    printf("game state: %d, life left: %d\n", s->round, s->life);
    if(s->round == 0){
        response = "VICT";
        // Player has won, remove them from the game?
        // for now, tear down the entire game
        tear_server = true;
    }else{
        response = "PASS";
    }
    return response;
}


/**
 * function to be called to check whether the client has lost or not
*/
char *check_lose(STATE *s){
    char *response = malloc(4);
    //We have completed the final round
    printf("game state: %d, life left: %d\n", s->round, s->life);
    if(s->life == 0){
        response = "ELIM";
        tear_server = true;
    }else{
        response = "FAIL";
    }
    return response;
}



/**
 * Function receives dice roll result and client guess from interpreter, 
 * returns appropriate response and changes game state.
*/
char *Responder(int * dice, enum valid_move m , STATE *s){
    char *response = malloc(4); // Appropriate response for client (always 4 characters)
    //s.round++;
    switch(m){
        case Eve :
            if((dice[0] + dice[1])%2 == 0){
                printf("correct guess, dice results are even\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice roll is odd\n");
                s->life--;
                response = check_lose(s);
            }
            break;

        case Odd :
            if((dice[0] + dice[1])%2 != 0){
                printf("correct guess, dice results are odd\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice roll is even\n");
                s->life--;
                response = check_lose(s);
            }
            break;

        case Dou :
            if(dice[0] == dice[1]){
                printf("correct guess, dice results are doubles\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice roll is doubles\n");
                s->life--;
                response = check_lose(s);
            }
            break;

        case One :
            if(dice[0] == 1 || dice[1] == 1){
                printf("correct guess, a dice is 1\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice is not 1\n");
                s->life--;
                response = check_lose(s);
            }            
            break;

        case Two :
            if(dice[0] == 2 || dice[1] == 2){
                printf("correct guess, a dice is 2\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice is not 2\n");
                s->life--;
                response = check_lose(s);
            }            
            break;

        case Thr :
            if(dice[0] == 3 || dice[1] == 3){
                printf("correct guess, a dice is 3\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice is not 3\n");
                s->life--;
                response = check_lose(s);
            }             
            break;

        case Fou :
            if(dice[0] == 4 || dice[1] == 4){
                printf("correct guess, a dice is 4\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice is not 4\n");
                s->life--;
                response = check_lose(s);
            }
            break;

        case Fiv :
            if(dice[0] == 5 || dice[1] == 5){
                printf("correct guess, a dice is 5\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice is not 5\n");
                s->life--;
                response = check_lose(s);
            }             
            break;

        case Six :
            if(dice[0] == 6 || dice[1] == 6){
                printf("correct guess, a dice is 6\n");
                s->round--;
                response = check_win(s);
            }else{
                printf("incorrect guess, dice is not 6\n");
                s->life--;
                response = check_lose(s);
            }             
            break;
    }
    return response;
}






int main (int argc, char *argv[]) {
    
    // Initialising server 
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int server_fd, client_fd, err, opt_val;
    struct sockaddr_in server, client;
    char *buf;


    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

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

    err = listen(server_fd, 128);
    if (err < 0){
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    } 

    printf("Server is listening on %d\n", port);


    
    // Initialise state variables
    STATE s;
    s.life = 3;
    s.Number_of_Players = 1;
    s.round = 3;
    

    // for testing code to prevent it from going into an infinite loop
    int test_counter = 0;


    while (true) {
        printf("In first while loop");
        socklen_t client_len = sizeof(client);




        // Will block until a connection is made
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);



        if (client_fd < 0) {
            fprintf(stderr,"Could not establish new connection\n");
            //SEND REJECT
            exit(EXIT_FAILURE);
        } 
        
        // For a single client - specify the client_id
        // (Specified in python script)
        int client_id = 231;


        /**
         * Initialisation of the client must happen outside the while loop.
         * Later this will need to be its own function
        */
        buf = calloc(BUFFER_SIZE, sizeof(char)); // Clear our buffer so we don't accidentally send/print garbage

        // Try to receiving INIT from client
        int read = recv(client_fd, buf, BUFFER_SIZE, 0);

        if (read < 0){
            // We didnt hear from the client
            fprintf(stderr,"Client read failed\n");

            // Teardown server we just created
            exit(EXIT_FAILURE);
        }



        // Check that the client sent an INIT call
        if (strstr(buf, "INIT") != NULL) {  // Check if the message contained 'INIT'
            // Assume that if they call INIT, they are allowed to connect

            // server gets rid of what was in the buffer to send our own
            buf[0] = '\0';

            // If we can connect, send a welcome to the client
            sprintf(buf, "WELCOME,%d", client_id);
            err = send(client_fd, buf, strlen(buf), 0);

            printf("Sending to client: %s\n",buf);
            sleep(5); //Wait 5 seconds

        } else {
            buf[0] = '\0';
            sprintf(buf, "REJECT");
            send(client_fd, buf, strlen(buf), 0);

            fprintf(stderr,"Client send comment but is not initialised - Rejected\n");
        }



        // Get info from client
        // save output of interp info 
        // Roll dice and save variable 
        // responder compares dice with client move
        // Read responce move from client
        while (true) {  
            printf("In second while loop");

            // Clear our buffer so we don't accidentally send/print garbage
            buf = calloc(BUFFER_SIZE, sizeof(char)); 

            // Read message from the client
            read = recv(client_fd, buf, BUFFER_SIZE, 0);
            if (read < 0){
                fprintf(stderr,"Client read failed\n");
                exit(EXIT_FAILURE);
            }


            // Relay what client said and call interpreter
            printf("client sent %s\n", buf);
            move m = client_responce(buf);
            
            // clearing buffer so we can send our responce
            buf[0] = '\0';

            // Print client name and response to the buffer
            sprintf(buf,"%d,%s", client_id, Responder(rollDice(),m, &s));
            printf("server will send: %s\n", buf);

            err = send(client_fd, buf, strlen(buf), 0); // Send our response
            sleep(5); //Wait 5 seconds
            test_counter ++;

            if(tear_server){
                tear(server_fd);
            }

            //Test counter for stopping infinite loops
            printf("test counter is: %d --------------\n", test_counter);
            if(test_counter == 10){
                fprintf(stderr,"infinite loop\n");
                exit(EXIT_FAILURE);
            }

            if (err < 0){
                    fprintf(stderr,"Client write failed\n");
                    exit(EXIT_FAILURE);
            }
        }
        free(buf);

    }
}

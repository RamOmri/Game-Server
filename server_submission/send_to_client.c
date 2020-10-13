#include "br_server.h"

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

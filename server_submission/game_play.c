#include "br_server.h"


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
                if(debug){printf("there was a draw\n");}
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
            if(debug){printf("eliminating client\n");}
            send_to_client(client[i], s->client_id[i], "ELIM");
            // mark that they have been eliminated
            s->life[i] = -1;
        }
    }
    
    // If there is only 1 player remaining, they should win
    if(clients_playing == 1){
        if(debug){printf("one client playing\n");}
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
            send_to_client(client, s->client_id[cli_no], "ELIM");
            s->life[cli_no] = -1;
        }else{
            // Relay what client said and call interpreter
            printf("client sent %s\n", buf);
            MOVE m = client_responce(buf);
            
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

#include "br_server.h"





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
char *Responder(int * dice, MOVE m , STATE *s, int cli_no){
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
MOVE client_responce(char* buf){
    
    if (strstr(buf, "EVEN") != NULL) {
        return Eve;
    }else if (strstr(buf, "ODD") != NULL) {
        return Odd;
    } else if(strstr(buf, "DOUB") != NULL){
        return Dou;
    }else if(strstr(buf, "CON,1") != NULL){
        return One;
    }else if(strstr(buf, "CON,2") != NULL){
        return Two;
    }else if(strstr(buf, "CON,3") != NULL){
        return Thr;
    }else if(strstr(buf, "CON,4") != NULL){
        return Fou;
    }else if(strstr(buf, "CON,5") != NULL){
        return Fiv;
    }else if(strstr(buf, "CON,6") != NULL){
        return Six;
    } else{
        // Can not recognise the message being sent
        // Interpreting this as different to cheating as
        // the player has not attempted to make a move
        return Inv;
    }
}

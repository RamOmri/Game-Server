#include "br_server.h"
/**
 * Based on code found at https://github.com/mafintosh/echo-servers.c (Copyright (c) 2014 Mathias Buus)
 * Copyright 2019 Nicholas Pritchard, Ryan Bunney
 **/

/**
 * @brief An implementation of a TCP network that utilises the select function
 * to iterate over all the clients and evaluate the state of the game
 */


/*
 CITS3002 Project 2019
 Name(s):             student-name1 (, student-name2)
 Student number(s):   student-number-1 (, student-number-2)
 Date:                date-of-submission
 */

bool debug = true;
bool tear_server = false;


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
    int server_fd;

    
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
        fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL, 0) | O_NONBLOCK);
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

#include "br_server.h"


struct timeval timeout;
fd_set set;

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
 * Initialise the clients and check if they have sent an INIT.
 * If they have sent an INIT, send a welcome in return,
 * otherwise, send them a REJECT message
 *
 * @param client: the client file descriptor
 * @param client_id: the randomly generated id
 */
bool init_client(int client, int client_id){
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

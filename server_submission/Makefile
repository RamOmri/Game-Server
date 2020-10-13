#
NAME	= br_server

CC	= clang


OBJ	= br_server.o setup_server.o game_play.o\
		send_to_client.o \
		receive_from_client.o 


$(NAME) :	$(OBJ)
	$(CC)  -o $(NAME) $(OBJ)


%.o :	%.c $(NAME).h
	$(CC)  -c $<


clean:
	rm -f $(NAME) *.o

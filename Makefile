CC = g++
CFLAGS = -std=c++2a -pedantic -Werror -Wall -Wextra -g
OBJS = mytftpclient.o argumentparser.o

mytftpclient: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o mytftpclient

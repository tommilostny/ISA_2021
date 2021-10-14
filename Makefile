CC = g++
CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra -g
OBJS = mytftpclient.o argumentparser.o

mytftpclient: $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o mytftpclient
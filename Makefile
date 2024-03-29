# ISA project 2021 (TFTP client) Makefile.
# Author: Tomáš Milostný (xmilos02)

CC = g++
CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra -Werror
OBJS = mytftpclient.o ArgumentParser.o Tftp.o StampMessagePrinter.o

# Compile mytftpclient and its dependencies.
mytftpclient: $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@

run: mytftpclient
	sudo ./$^

# Delete built files.
clean:
	rm -f *.o mytftpclient xmilos02.tar

# Create .tar archive for project submission.
tar:
	tar -cf xmilos02.tar *.cpp *.hpp Makefile manual.pdf README.md

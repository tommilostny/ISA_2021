# ISA project 2021 (TFTP client) Makefile.
# Author: Tomáš Milostný (xmilos02)

CC = g++
CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra
OBJS = mytftpclient.o argumentparser.o tftp.o

# Set default build mode.
default: debug

# Compile mytftpclient and its dependencies.
mytftpclient: $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@

# Build mytftpclient with debugging flags.
debug: CXXFLAGS += -g -DDEBUG
debug: mytftpclient

# Build mytftpclient for release.
release: CXXFLAGS += -Werror
release: mytftpclient

# Delete built files.
clean:
	rm -f *.o mytftpclient xmilos02.tar

# Create .tar archive for project submission.
tar:
	tar -cf xmilos02.tar *.cpp *.hpp Makefile manual.pdf README

valgrind: debug
	valgrind ./mytftpclient < test-inputs.txt

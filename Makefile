# ‘make all' compiles all files and create executables
CC = g++
CFLAGS = -o
all:
	$(CC) $(CFLAGS) serverM serverM.cpp
	$(CC) $(CFLAGS) serverC serverC.cpp
	$(CC) $(CFLAGS) serverEE serverEE.cpp
	$(CC) $(CFLAGS) serverCS serverCS.cpp
	$(CC) $(CFLAGS) client client.cpp
clean:
	$(RM) serverM
	$(RM) serverC
	$(RM) serverEE
	$(RM) serverCS
	$(RM) client


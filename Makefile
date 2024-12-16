CC = clang
CFLAGS = -Wall -g -lpthread

OUTPUT = echo_server

SRCS = echo_server.c

# Detect if we're running in PowerShell or WSL
ifeq ($(shell uname -o 2>/dev/null), Msys)
    MAKECMD = wsl make
    RUNCMD = wsl ./$(OUTPUT)
else
    MAKECMD = $(MAKE)
    RUNCMD = ./$(OUTPUT)
endif

all:
	$(MAKECMD) $(OUTPUT)

$(OUTPUT): $(SRCS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SRCS)

clean:
	rm -f $(OUTPUT)

run: $(OUTPUT)
	$(RUNCMD) -p 8080

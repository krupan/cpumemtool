CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Werror -fsanitize=undefined \
	-fsanitize=address -fno-sanitize-recover

OUTDIR=bin

$(OUTDIR)/cpumemd: cpumemd.cpp
	g++ $(CFLAGS) -o $(OUTDIR)/cpumemd cpumemd.cpp -lboost_system -lpthread

clean:
	rm $(OUTDIR)/*

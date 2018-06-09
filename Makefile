CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Werror -fsanitize=undefined \
	-fsanitize=address -fno-sanitize-recover

OUTDIR=bin

all: $(OUTDIR)/cpumemd $(OUTDIR)/cpumemtool

$(OUTDIR)/cpumemd: cpumemd.cpp
	g++ $(CFLAGS) -o $(OUTDIR)/cpumemd cpumemd.cpp -lboost_system -lpthread

$(OUTDIR)/cpumemtool: cpumemtool.cpp
	g++ $(CFLAGS) -o $(OUTDIR)/cpumemtool cpumemtool.cpp -lboost_system -lpthread

clean:
	rm $(OUTDIR)/*

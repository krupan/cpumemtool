cpumemd: cpumemd.cpp
	g++ -g -Wall -Wextra -Wno-unused-parameter -Werror -fsanitize=undefined -fsanitize=address -fno-sanitize-recover -o bin/cpumemd cpumemd.cpp -lboost_system -lpthread 


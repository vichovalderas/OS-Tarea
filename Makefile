CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -lpthread

planificador: main.cpp
	$(CXX) $(CXXFLAGS) -o planificador main.cpp

clean:
	rm -f planificador

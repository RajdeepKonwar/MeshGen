CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall -Wextra -pedantic

SRC = EurekaGen.cpp EurekaWriter.cpp ../GeoGen/GeoMath.cpp
OBJ = $(SRC:.cpp = .o)

EurekaGen: $(OBJ)
	$(CXX) $(CXXFLAGS) -o EurekaGen $(OBJ)

.PHONY: clean

clean:
	rm -f EurekaGen *.o

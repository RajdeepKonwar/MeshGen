CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall -Wextra -pedantic

SRC = GeoGen.cpp GeoMath.cpp GeoWriter.cpp
OBJ = $(SRC:.cpp = .o)

GeoGen: $(OBJ)
	$(CXX) $(CXXFLAGS) -o GeoGen $(OBJ)

.PHONY: clean

clean:
	rm -f GeoGen *.o

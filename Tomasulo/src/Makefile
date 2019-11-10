
CXX = g++
SRC = procsim.cpp procsim_driver.cpp
INCLUDE = procsim.hpp
CXXFLAGS := -g -Wall -Wextra -std=c++11

build:
	$(CXX) $(CXXFLAGS) $(SRC) $(INCLUDE) -o procsim

clean:
	rm -f procsim *.o

# This looks for a report pdf in the top level directory of the project
.PHONY: submit

submit:
	tar -cvzf ../../project2-submit.tar.gz procsim.cpp procsim_driver.cpp procsim.hpp \
				Makefile $(PWD)/../../report.pdf

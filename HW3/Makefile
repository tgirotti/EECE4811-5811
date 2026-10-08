CXX := g++

CXXFLAGS := -O0 -Wall -Wextra -Wpedantic -Werror \
            -Wconversion -Wshadow -Wformat=2 -Wundef \
            -Wnull-dereference -Wdouble-promotion \
            -Wimplicit-fallthrough \
			-pthread -ggdb -gdwarf

TARGET := main
SRC := main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
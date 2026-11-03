#CC := clang++-22
CXX := clang++-22
TIDY := clang-tidy-22

CXXFLAGS:= -O3 --std=c++20 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security

SRC := l1_benchmark.cpp
HEADERS :=

TARGET := main
DEBUG_TARGET := main-debug

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(DEBUG_TARGET): $(SRC) $(HEADERS)
	$(CXX) $(DEBUGFLAGS) $(SRC) -o $(DEBUG_TARGET)

run: $(TARGET)
	./$(TARGET)

tidy:
	$(TIDY) $(SRC) \
		--allow-enabling-analyzer-alpha-checkers \
		--checks='*,-llvmlibc*,-altera*,-google-runtime*,-misc-use-internal-linkage,-google-readability*,-readability-identifier*,-readability-braces*' \
		-- $(filter-out -Wl%,$(CXXFLAGS))

clean:
	rm -f $(TARGET) $(DEBUG_TARGET)

.PHONY: run clean tidy

#CC := clang++-22
CXX := clang++-22
TIDY := clang-tidy-22

CXXFLAGS:= -O2 --std=c++20 -Wall -Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough \
-Werror=format-security \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
-D_GLIBCXX_ASSERTIONS \
-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST \
-fstrict-flex-arrays=3 \
-fstack-clash-protection -fstack-protector-strong \
-Wl,-z,nodlopen -Wl,-z,noexecstack \
-Wl,-z,relro -Wl,-z,now \
-Wl,--as-needed -Wl,--no-copy-dt-needed-entries

TARGET := main
SRC := main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

tidy:
	$(TIDY) $(SRC) \
		--allow-enabling-analyzer-alpha-checkers \
		--checks='*,-llvmlibc*,-altera*,-google-runtime*,-misc-use-internal-linkage,-google-readability*,-readability-identifier*,-readability-braces*' \
		-- $(filter-out -Wl%,$(CXXFLAGS))

clean:
	rm -f $(TARGET)

.PHONY: run clean tidy

# Output binary name
bin=inspector

# Set the following to '0' to disable log messages:
DEBUG ?= 1

# Compiler/linker flags
CFLAGS += -g -lm -Wall -Werror -DDEBUG=$(DEBUG)
LDFLAGS +=

# Source C files
src=inspector.c
obj=$(src:.c=.o)

# Makefile recipes --
$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

docs: Doxyfile
	doxygen

clean:
	rm -f $(bin) $(obj)
	rm -rf docs


# Individual dependencies --
inspector.o: inspector.c debug.h


# Tests --

test: inspector ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	git submodule update --init --remote

testclean:
	rm -rf tests

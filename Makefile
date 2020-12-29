CC            := clang
CCFLAGS       := -Wall -Wextra -Werror -pedantic -std=c17 -g
ASSIGNMENT    := a3
.DEFAULT_GOAL := help

.PHONY: reset clean bin lib all run test help

clean:			## cleans up project folder
	@echo "[\033[36mINFO\033[0m] Cleaning up folder..."
	rm -f $(ASSIGNMENT)
	rm -f $(ASSIGNMENT).so
	rm -rf result.json
	rm -rf result.html
	rm -rf ./tmp

bin:			## compiles project to executable binary
	@echo "[\033[36mINFO\033[0m] Compiling binary..."
	$(CC) $(CCFLAGS) -o $(ASSIGNMENT) $(ASSIGNMENT).c framework.c
	chmod +x $(ASSIGNMENT)


lib:			## compiles project to shared library
	@echo "[\033[36mINFO\033[0m] Compiling library..."
	$(CC) $(CCFLAGS) -shared -fPIC -o $(ASSIGNMENT).so $(ASSIGNMENT).c framework.c

all: clean bin lib	## all of the above

run: all		## runs the project with default config
	@echo "[\033[36mINFO\033[0m] Executing binary..."
	./$(ASSIGNMENT) ./config/config_02.bin

test: all		## runs public testcases on the project
	@echo "[\033[36mINFO\033[0m] Executing testrunner..."
	./testrunner -c test.toml
	
testbychar: all             ## runs public testcases on the project compares output char by char
	@echo "[\033[36mINFO\033[0m] Executing testrunner..."
	./testrunner -c test.toml -m char

testbyword: all             ## runs public testcases on the project compares output word by word
	@echo "[\033[36mINFO\033[0m] Executing testrunner..."
	./testrunner -c test.toml -m word
	
testprint: all             ## runs public testcases on the project compares output line by line, prints to terminal
	@echo "[\033[36mINFO\033[0m] Executing testrunner..."
	./testrunner -c test.toml -v
	
help:			## prints the help text
	@echo "Usage: make \033[36m<TARGET>\033[0m"
	@echo "Available targets:"
	@awk -F':.*?##' '/^[a-zA-Z_-]+:.*?##.*$$/{printf "  \033[36m%-10s\033[0m%s\n", $$1, $$2}' $(MAKEFILE_LIST)
	

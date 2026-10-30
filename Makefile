.PHONY: all build test valgrind run docs clean rebuild

all: build

build:
	cmake -S . -B build
	cmake --build build -j

test: build
	ctest --test-dir build --output-on-failure

valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build/tividy_tests

run: build
	./build/tividy

docs:
	doxygen Doxyfile
	@echo "----------------------------------------------------"
	@echo "Documentation generated: file:TIVIDY-Prac6/html/index.html"
	@echo "----------------------------------------------------"

clean:
	rm -rf build html 

rebuild: clean build
.RECIPEPREFIX = >
.PHONY: all build test run clean docs rebuild

all: build

build:
> cmake -S . -B build
> cmake --build build -j

test: build
> ctest --test-dir build --output-on-failure

run: build
> ./build/tividy

docs:
> doxygen Doxyfile

clean:
> rm -rf build html latex

rebuild: clean build

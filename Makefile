.PHONY: all build build-no-tui test test-no-tui valgrind run docs clean rebuild

all: build

# Builds the TUI when Curses is installed; otherwise builds engine + tests.
build:
	mkdir -p build
	cd build && cmake ..
	cmake --build build

# Explicit portable build that does not even search for Curses.
build-no-tui:
	mkdir -p build-no-tui
	cd build-no-tui && cmake -DTIVIDY_BUILD_TUI=OFF ..
	cmake --build build-no-tui

test: build
	cd build && ctest --output-on-failure

test-no-tui: build-no-tui
	cd build-no-tui && ctest --output-on-failure

valgrind: build
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build/tividy_tests

run: build
	@if [ -x ./build/tividy ]; then \
		./build/tividy; \
	else \
		echo "The TUI was not built. Install libncurses-dev and rerun 'make build', or use the engine/tests-only build."; \
		exit 1; \
	fi

docs:
	doxygen Doxyfile
	@echo "----------------------------------------------------"
	@echo "Documentation generated: html/index.html"
	@echo "----------------------------------------------------"

clean:
	rm -rf build build-no-tui html latex

rebuild: clean build

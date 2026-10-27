// #include <iostream>
// #include <limits>
// #include <stdexcept>

// #include "engine/Console.h"

// void runHospitalAdmissionScenario();
// void runUniversityComplaintScenario();

// int main() {
//     using namespace tividy::console;

//     while (true) {
//         std::cout << BOLD << "\n========================================\n";
//         std::cout << "  TIVIDY WORKFLOW ENGINE - MOCK TUI\n";
//         std::cout << "  (progress demo: hard-coded scenarios)\n";
//         std::cout << "========================================\n" << RESET;
//         std::cout << CYAN    << "1. Hospital admission workflow\n" << RESET;
//         std::cout << MAGENTA << "2. University complaint workflow\n" << RESET;
//         std::cout << "0. Exit\n";
//         std::cout << "Select a scenario: ";

//         int choice;
//         if (!(std::cin >> choice)) {
//             if (std::cin.eof()) return 0;
//             std::cin.clear();
//             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//             std::cout << RED << "Please enter 0, 1 or 2.\n" << RESET;
//             continue;
//         }

//         try {
//             if (choice == 1) {
//                 runHospitalAdmissionScenario();
//             } else if (choice == 2) {
//                 runUniversityComplaintScenario();
//             } else if (choice == 0) {
//                 std::cout << "Exiting TIVIDY demo.\n";
//                 return 0;
//             } else {
//                 std::cout << RED << "Please select 0, 1 or 2.\n" << RESET;
//             }
//         } catch (const std::exception& error) {
//             std::cerr << RED << "Scenario error: " << error.what() << RESET << '\n';
//         } catch (...) {
//             std::cerr << RED << "Scenario error: an unexpected internal error occurred." << RESET << '\n';
//         }
//     }
// }


#include <ncurses.h>

#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

void runHospitalAdmissionScenario();
void runUniversityComplaintScenario();

namespace {

enum ColorPair {
    PAIR_TITLE = 1,
    PAIR_SELECTED,
    PAIR_NORMAL,
    PAIR_BORDER,
    PAIR_FOOTER
};

void initColors() {
    start_color();
    init_pair(PAIR_TITLE,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(PAIR_SELECTED, COLOR_BLACK,   COLOR_CYAN);  // reversed = highlighted row
    init_pair(PAIR_NORMAL,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(PAIR_BORDER,   COLOR_MAGENTA, COLOR_BLACK);
    init_pair(PAIR_FOOTER,   COLOR_YELLOW,  COLOR_BLACK);
}

/**
 * Draws a centred, bordered menu and lets the user move with the arrow
 * keys. Returns the 0-based index chosen, or -1 if the user quit.
 */
int runMenu(const std::vector<std::string>& items) {
    int highlight = 0;
    int choice = -1;

    int rows = 0;
    int cols = 0;
    getmaxyx(stdscr, rows, cols);

    int boxWidth  = 48;
    int boxHeight = static_cast<int>(items.size()) + 6;
    int startY = (rows - boxHeight) / 2;
    int startX = (cols - boxWidth) / 2;
    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    WINDOW* win = newwin(boxHeight, boxWidth, startY, startX);
    keypad(win, TRUE);

    while (true) {
        werase(win);

        wattron(win, COLOR_PAIR(PAIR_BORDER));
        box(win, 0, 0);
        wattroff(win, COLOR_PAIR(PAIR_BORDER));

        wattron(win, COLOR_PAIR(PAIR_TITLE) | A_BOLD);
        mvwprintw(win, 1, (boxWidth - 23) / 2, "TIVIDY WORKFLOW ENGINE");
        mvwprintw(win, 2, (boxWidth - 9) / 2, "MOCK TUI");
        wattroff(win, COLOR_PAIR(PAIR_TITLE) | A_BOLD);

        for (std::size_t i = 0; i < items.size(); ++i) {
            int y = 4 + static_cast<int>(i);
            if (static_cast<int>(i) == highlight) {
                wattron(win, COLOR_PAIR(PAIR_SELECTED));
                mvwprintw(win, y, 3, "%-*s", boxWidth - 6, items[i].c_str());
                wattroff(win, COLOR_PAIR(PAIR_SELECTED));
            } else {
                wattron(win, COLOR_PAIR(PAIR_NORMAL));
                mvwprintw(win, y, 3, "%-*s", boxWidth - 6, items[i].c_str());
                wattroff(win, COLOR_PAIR(PAIR_NORMAL));
            }
        }

        wattron(win, COLOR_PAIR(PAIR_FOOTER));
        mvwprintw(win, boxHeight - 2, 2, "Arrows to move, Enter to select, q to quit");
        wattroff(win, COLOR_PAIR(PAIR_FOOTER));

        wrefresh(win);

        int key = wgetch(win);
        if (key == KEY_UP) {
            highlight = (highlight == 0) ? static_cast<int>(items.size()) - 1 : highlight - 1;
        } else if (key == KEY_DOWN) {
            highlight = (highlight == static_cast<int>(items.size()) - 1) ? 0 : highlight + 1;
        } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            choice = highlight;
            break;
        } else if (key == 'q' || key == 'Q') {
            choice = -1;
            break;
        }
    }

    delwin(win);
    return choice;
}

/** Pauses, outside curses, until the user presses Enter. */
void waitForEnter() {
    std::cout << "\nPress Enter to return to the menu...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * Leaves curses mode so a scenario can print normally (including the
 * existing ANSI colours from Console.h), waits for the user, then resumes
 * curses for the next menu draw.
 */
void runScenarioOutsideCurses(void (*scenario)()) {
    endwin();

    try {
        scenario();
    } catch (const std::exception& error) {
        std::cerr << "Scenario error: " << error.what() << '\n';
    } catch (...) {
        std::cerr << "Scenario error: an unexpected internal error occurred.\n";
    }

    waitForEnter();
    refresh();  // repaints the curses screen, resuming the TUI
}

/**
 * Leaves curses mode and opens the Doxygen HTML docs (run `make docs`
 * first) in the default browser via xdg-open, then resumes curses.
 */
void viewDocumentation() {
    endwin();

    const std::string docsPath = "html/index.html";
    std::ifstream check(docsPath.c_str());
    if (!check.good()) {
        std::cout << "Documentation not found at " << docsPath << ".\n";
        std::cout << "Run 'make docs' first, then try again.\n";
    } else {
        std::string command = "xdg-open " + docsPath + " >/dev/null 2>&1 &";
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cout << "Could not launch a browser automatically.\n";
            std::cout << "Open it manually: file://" << docsPath << '\n';
        } else {
            std::cout << "Opening documentation in your browser...\n";
        }
    }

    waitForEnter();
    refresh();
}

} // namespace

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    if (has_colors()) {
        initColors();
    }

    std::vector<std::string> items;
    items.push_back("Hospital admission workflow");
    items.push_back("University complaint workflow");
    items.push_back("View documentation");
    items.push_back("Exit");

    while (true) {
        int choice = runMenu(items);

        if (choice == 0) {
            runScenarioOutsideCurses(runHospitalAdmissionScenario);
        } else if (choice == 1) {
            runScenarioOutsideCurses(runUniversityComplaintScenario);
        } else if (choice == 2) {
            viewDocumentation();
        } else {
            break;  // Exit selected, or 'q' pressed
        }
    }

    endwin();
    std::cout << "Exiting TIVIDY demo.\n";
    return 0;
}
/**
 * @file main.cpp
 * @brief TIVIDY mock TUI with automatic and interactive modes for both
 *        industry scenarios, plus generated documentation access.
 */
#include <ncurses.h>

#include <unistd.h>

#include <chrono>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

void runHospitalAdmissionScenario();
void runHospitalAdmissionInteractiveScenario();
void runUniversityComplaintScenario();
void runUniversityComplaintInteractiveScenario();

namespace {

// Accent colours. The values match ncurses' COLOR_RED..COLOR_WHITE, and each is
// also its colour-pair number (see initColors).
enum Accent {
    ACCENT_RED = 1,
    ACCENT_GREEN,
    ACCENT_YELLOW,
    ACCENT_BLUE,
    ACCENT_MAGENTA,
    ACCENT_CYAN,
    ACCENT_WHITE
};

enum Action {
    ACTION_HOSPITAL_AUTOMATIC,
    ACTION_HOSPITAL_INTERACTIVE,
    ACTION_UNIVERSITY_AUTOMATIC,
    ACTION_UNIVERSITY_INTERACTIVE,
    ACTION_DOCS,
    ACTION_EXIT
};

struct MenuItem {
    std::string label;                  // shown in the menu and as the panel title
    std::string tag;                    // small caption under the label
    std::string verb;                   // completes "Press Enter to ..."
    int accent;                         // colour used for this item
    Action action;
    std::vector<std::string> details;   // bullet points for the right-hand panel
};

struct StatusMessage {
    std::string text;
    int accent;
};

const int MIN_ROWS = 20;
const int MIN_COLS = 72;

// ------------------------------------------------------------ small helpers

/** Colours use the terminal's own background, so no black patches appear. */
void initColors() {
    if (!has_colors()) return;
    start_color();
    const short background = (use_default_colors() == OK) ? -1 : COLOR_BLACK;
    for (short c = ACCENT_RED; c <= ACCENT_WHITE; ++c) {
        init_pair(c, c, background);
    }
}

int pairAttr(int accent) { return COLOR_PAIR(accent); }

unsigned long animationTick() {
    const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    return static_cast<unsigned long>(ms / 120);
}

std::string clockText() {
    const std::time_t now = std::time(nullptr);
    const std::tm* local = std::localtime(&now);
    if (local == nullptr) return std::string();
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
    return buffer;
}

/** Greedy word wrap. */
std::vector<std::string> wrapText(const std::string& text, std::size_t width) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word, line;
    while (words >> word) {
        if (line.empty()) {
            line = word;
        } else if (line.size() + 1 + word.size() <= width) {
            line += " " + word;
        } else {
            lines.push_back(line);
            line = word;
        }
    }
    if (!line.empty() || lines.empty()) lines.push_back(line);
    return lines;
}

// --------------------------------------------------------- drawing helpers

void putText(int y, int x, const std::string& text, int attr = 0) {
    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);
    if (y < 0 || y >= rows || x < 0 || x >= cols - 1) return;
    const std::string clipped = text.substr(0, static_cast<std::size_t>(cols - 1 - x));
    attron(attr);
    mvaddstr(y, x, clipped.c_str());
    attroff(attr);
}

void putCentered(int y, int cols, const std::string& text, int attr = 0) {
    const int x = (cols - static_cast<int>(text.size())) / 2;
    putText(y, x < 0 ? 0 : x, text, attr);
}

void drawRule(int y, int x, int width, int attr) {
    attron(attr);
    for (int i = 0; i < width; ++i) mvaddch(y, x + i, ACS_HLINE);
    attroff(attr);
}

void drawFrame(int y, int x, int h, int w, int attr, const std::string& title) {
    attron(attr);
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + w - 1, ACS_URCORNER);
    mvaddch(y + h - 1, x, ACS_LLCORNER);
    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);
    for (int i = 1; i < w - 1; ++i) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + h - 1, x + i, ACS_HLINE);
    }
    for (int j = 1; j < h - 1; ++j) {
        mvaddch(y + j, x, ACS_VLINE);
        mvaddch(y + j, x + w - 1, ACS_VLINE);
    }
    attroff(attr);
    if (!title.empty()) putText(y, x + 2, " " + title + " ", attr | A_BOLD);
}

/** Decorative strip echoing the State pattern lifecycle; one stage lights up at a time. */
void drawPipeline(int y, int cols, int accent, unsigned long tick) {
    static const char* const stages[] = {"Submitted", "Active", "In Progress", "Resolved"};
    const int count = 4;
    const std::string arrow = " --> ";
    int total = 0;
    for (int i = 0; i < count; ++i) total += static_cast<int>(std::string(stages[i]).size()) + 4;
    total += (count - 1) * static_cast<int>(arrow.size());

    int x = (cols - total) / 2;
    if (x < 0) x = 0;
    const int active = static_cast<int>((tick / 7) % count);
    for (int i = 0; i < count; ++i) {
        const std::string cell = std::string("[ ") + stages[i] + " ]";
        int attr = A_DIM;
        if (i < active) attr = pairAttr(ACCENT_GREEN);
        if (i == active) attr = pairAttr(accent) | A_REVERSE | A_BOLD;
        putText(y, x, cell, attr);
        x += static_cast<int>(cell.size());
        if (i + 1 < count) {
            putText(y, x, arrow, A_DIM);
            x += static_cast<int>(arrow.size());
        }
    }
}

void drawStatusBar(int rows, int cols, const std::string& hints, unsigned long tick) {
    attron(A_REVERSE);
    for (int x = 0; x < cols - 1; ++x) mvaddch(rows - 1, x, ' ');
    attroff(A_REVERSE);
    putText(rows - 1, 0, hints, A_REVERSE);
    const std::string right = std::string(1, "|/-\\"[tick % 4]) + " " + clockText() + " ";
    putText(rows - 1, cols - static_cast<int>(right.size()) - 2, right, A_REVERSE);
}

void drawScreen(const std::vector<MenuItem>& items, std::size_t selected,
                const StatusMessage& status) {
    int rows = 0, cols = 0;
    getmaxyx(stdscr, rows, cols);
    erase();

    if (rows < MIN_ROWS || cols < MIN_COLS) {
        putText(0, 0, "Terminal too small: need at least 72x20. Resize, or press q.",
                pairAttr(ACCENT_YELLOW));
        refresh();
        return;
    }

    const MenuItem& current = items[selected];
    const unsigned long tick = animationTick();

    // Header
    putCentered(0, cols, "T I V I D Y", pairAttr(current.accent) | A_BOLD);
    putCentered(1, cols, "Workflow Management System  -  Mock TUI", A_DIM);
    drawPipeline(2, cols, current.accent, tick);
    drawRule(3, 1, cols - 2, A_DIM);

    // Body: menu on the left, details on the right
    const int top = 4;
    const int bodyH = rows - top - 2;
    const int leftW = 34;
    drawFrame(top, 1, bodyH, leftW, pairAttr(current.accent), "Menu");

    for (std::size_t i = 0; i < items.size(); ++i) {
        const int y = top + 1 + static_cast<int>(i) * 2;
        const bool isSelected = (i == selected);
        std::string line = std::string(isSelected ? "> " : "  ") + std::to_string(i + 1) +
                           "  " + items[i].label;
        line.resize(static_cast<std::size_t>(leftW - 4), ' ');
        if (isSelected) {
            putText(y, 3, line, pairAttr(items[i].accent) | A_REVERSE | A_BOLD);
        } else {
            putText(y, 3, line);
        }
        putText(y + 1, 8, items[i].tag, pairAttr(items[i].accent) | A_DIM);
    }

    const int panelX = leftW + 2;
    const int panelW = cols - panelX - 1;
    drawFrame(top, panelX, bodyH, panelW, pairAttr(current.accent), current.label);

    int y = top + 2;
    putText(y, panelX + 2, current.tag, pairAttr(current.accent) | A_BOLD);
    y += 2;
    const int lastRow = top + bodyH - 4;
    for (std::size_t d = 0; d < current.details.size(); ++d) {
        const std::vector<std::string> lines =
            wrapText(current.details[d], static_cast<std::size_t>(panelW - 6));
        for (std::size_t k = 0; k < lines.size() && y <= lastRow; ++k, ++y) {
            putText(y, panelX + 2, std::string(k == 0 ? "- " : "  ") + lines[k]);
        }
    }

    const bool pulse = (tick / 4) % 2 == 0;
    putText(top + bodyH - 2, panelX + 2,
            std::string(pulse ? ">> " : "   ") + "Press Enter to " + current.verb,
            pairAttr(current.accent) | A_BOLD);

    // Footer
    putText(rows - 2, 2, status.text, pairAttr(status.accent) | A_BOLD);
    drawStatusBar(rows, cols, " UP/DOWN move | ENTER select | 1-6 jump | Q quit", tick);
    refresh();
}

std::vector<MenuItem> buildMenu() {
    std::vector<MenuItem> items;

    MenuItem hospitalAutomatic;
    hospitalAutomatic.label = "Hospital - automatic";
    hospitalAutomatic.tag = "HEALTHCARE | AUTO";
    hospitalAutomatic.verb = "run the complete hospital scenario";
    hospitalAutomatic.accent = ACCENT_CYAN;
    hospitalAutomatic.action = ACTION_HOSPITAL_AUTOMATIC;
    hospitalAutomatic.details.push_back("Runs the original patient admission demonstration automatically.");
    hospitalAutomatic.details.push_back("You answer the urgent-admission question; the engine then assigns and completes the planned work.");
    hospitalAutomatic.details.push_back("Shows the normal or urgent dependency path and the final resolved result.");
    items.push_back(hospitalAutomatic);

    MenuItem hospitalInteractive;
    hospitalInteractive.label = "Hospital - interactive";
    hospitalInteractive.tag = "HEALTHCARE | LIVE";
    hospitalInteractive.verb = "create an editable hospital instance";
    hospitalInteractive.accent = ACCENT_CYAN;
    hospitalInteractive.action = ACTION_HOSPITAL_INTERACTIVE;
    hospitalInteractive.details.push_back("Creates a hospital admission instance but leaves its work available for live management.");
    hospitalInteractive.details.push_back("Add, cancel and alter work; add participants; assign and complete tasks.");
    hospitalInteractive.details.push_back("Start additional independent instances and inspect each instance's audit history.");
    items.push_back(hospitalInteractive);

    MenuItem universityAutomatic;
    universityAutomatic.label = "University - automatic";
    universityAutomatic.tag = "EDUCATION | AUTO";
    universityAutomatic.verb = "run the complete university scenario";
    universityAutomatic.accent = ACCENT_MAGENTA;
    universityAutomatic.action = ACTION_UNIVERSITY_AUTOMATIC;
    universityAutomatic.details.push_back("Runs the original student complaint demonstration automatically.");
    universityAutomatic.details.push_back("The first three tasks are completed before you choose whether senior review is required.");
    universityAutomatic.details.push_back("Shows the normal path or handler-chain escalation, then completes the case.");
    items.push_back(universityAutomatic);

    MenuItem universityInteractive;
    universityInteractive.label = "University - interactive";
    universityInteractive.tag = "EDUCATION | LIVE";
    universityInteractive.verb = "create an editable university instance";
    universityInteractive.accent = ACCENT_MAGENTA;
    universityInteractive.action = ACTION_UNIVERSITY_INTERACTIVE;
    universityInteractive.details.push_back("Creates a university complaint instance for live workflow management.");
    universityInteractive.details.push_back("Add, cancel and alter work; add participants; assign and complete tasks.");
    universityInteractive.details.push_back("Start additional independent instances and inspect each instance's audit history.");
    items.push_back(universityInteractive);

    MenuItem docs;
    docs.label = "View documentation";
    docs.tag = "DOXYGEN";
    docs.verb = "open the documentation";
    docs.accent = ACCENT_YELLOW;
    docs.action = ACTION_DOCS;
    docs.details.push_back("Opens the generated class documentation in your browser.");
    docs.details.push_back("If it has not been generated yet, Doxygen is run for you first.");
    items.push_back(docs);

    MenuItem quit;
    quit.label = "Exit";
    quit.tag = "QUIT";
    quit.verb = "leave the demo";
    quit.accent = ACCENT_RED;
    quit.action = ACTION_EXIT;
    quit.details.push_back("Close the TIVIDY demo and return to the shell.");
    items.push_back(quit);

    return items;
}

// ---------------------------------------------------- leaving curses briefly

void waitForEnter() {
    std::cout << "\nPress Enter to return to the menu...";
    std::cout.flush();
    std::cin.clear();
    std::string ignored;
    std::getline(std::cin, ignored);
}

StatusMessage runScenarioOutsideCurses(const MenuItem& item, void (*scenario)()) {
    endwin();
    StatusMessage status;
    try {
        scenario();
        status.text = item.label + " finished.";
        status.accent = ACCENT_GREEN;
    } catch (const std::exception& error) {
        std::cerr << "Scenario error: " << error.what() << '\n';
        status.text = item.label + " failed: " + error.what();
        status.accent = ACCENT_RED;
    } catch (...) {
        std::cerr << "Scenario error: an unexpected internal error occurred.\n";
        status.text = item.label + " failed.";
        status.accent = ACCENT_RED;
    }
    waitForEnter();
    refresh();
    return status;
}

bool fileExists(const std::string& path) {
    std::ifstream file(path.c_str());
    return file.good();
}

/** Wraps text in single quotes so paths with spaces or brackets are safe in a shell. */
std::string shellQuote(const std::string& text) {
    std::string quoted = "'";
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\'') quoted += "'\\''";
        else quoted += text[i];
    }
    return quoted + "'";
}

/** Encodes only the characters that break a clickable link. */
std::string encodeForUrl(const std::string& path) {
    std::string out;
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (path[i] == ' ') out += "%20";
        else if (path[i] == '(') out += "%28";
        else if (path[i] == ')') out += "%29";
        else out += path[i];
    }
    return out;
}

std::string firstLineOf(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) return std::string();
    char buffer[PATH_MAX];
    std::string line;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) line = buffer;
    pclose(pipe);
    while (!line.empty() && (line[line.size() - 1] == '\n' || line[line.size() - 1] == '\r')) {
        line.erase(line.size() - 1);
    }
    return line;
}

/**
 * Opens a file in the default browser. xdg-open has no browser to talk to
 * inside WSL, so there the file is handed to Windows through explorer.exe.
 */
bool openInBrowser(const std::string& absolutePath) {
    const std::string quoted = shellQuote(absolutePath);
    if (std::getenv("WSL_DISTRO_NAME") != nullptr &&
        std::system("command -v explorer.exe >/dev/null 2>&1") == 0) {
        const std::string windowsPath = firstLineOf("wslpath -w " + quoted + " 2>/dev/null");
        if (!windowsPath.empty()) {
            // explorer.exe returns 1 even when it works, so its result is ignored.
            const int ignored = std::system(
                ("explorer.exe " + shellQuote(windowsPath) + " >/dev/null 2>&1").c_str());
            (void)ignored;
            return true;
        }
    }
    return std::system(("xdg-open " + quoted + " >/dev/null 2>&1").c_str()) == 0;
}

std::string findDocsIndex() {
    if (fileExists("docs/html/index.html")) return "docs/html/index.html";
    if (fileExists("html/index.html")) return "html/index.html";
    return std::string();
}

StatusMessage viewDocumentation(std::string& link) {
    endwin();
    StatusMessage status;
    status.accent = ACCENT_RED;

    std::string index = findDocsIndex();
    if (index.empty()) {
        std::cout << "Documentation not found. Building it with Doxygen...\n\n";
        std::cout.flush();
        const int built = std::system("doxygen Doxyfile");
        (void)built;
        index = findDocsIndex();
    }

    if (index.empty()) {
        std::cout << "\nCould not build the documentation. Check that doxygen is installed\n"
                     "and that you started the demo from the project folder (make run).\n";
        status.text = "Documentation could not be built.";
    } else {
        char resolved[PATH_MAX];
        const std::string absolute = (realpath(index.c_str(), resolved) != nullptr)
                                         ? std::string(resolved) : index;
        link = "file://" + encodeForUrl(absolute);
        std::cout << "\nDocumentation: " << link << '\n';
        if (openInBrowser(absolute)) {
            std::cout << "Opening it in your browser...\n";
            status.text = "Documentation opened in your browser.";
            status.accent = ACCENT_GREEN;
        } else {
            std::cout << "Could not open a browser automatically. Copy the link above.\n";
            status.text = "Documentation is ready: copy the link shown above.";
            status.accent = ACCENT_YELLOW;
        }
    }

    waitForEnter();
    refresh();
    return status;
}

}  // namespace

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);   // wake up every 100 ms so the header animation keeps moving
    initColors();

    const std::vector<MenuItem> items = buildMenu();
    std::size_t selected = 0;
    StatusMessage status;
    status.text = "Choose an industry and select automatic or interactive mode.";
    status.accent = ACCENT_YELLOW;
    std::string docsLink;
    bool running = true;

    while (running) {
        drawScreen(items, selected, status);
        const int key = getch();
        if (key == ERR) continue;   // timeout: just redraw

        bool activate = false;
        if (key == KEY_UP || key == 'k') {
            selected = (selected + items.size() - 1) % items.size();
        } else if (key == KEY_DOWN || key == 'j') {
            selected = (selected + 1) % items.size();
        } else if (key >= '1' && key < '1' + static_cast<int>(items.size())) {
            selected = static_cast<std::size_t>(key - '1');
            activate = true;
        } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            activate = true;
        } else if (key == 'q' || key == 'Q') {
            running = false;
        }

        if (!activate) continue;
        switch (items[selected].action) {
            case ACTION_HOSPITAL_AUTOMATIC:
                status = runScenarioOutsideCurses(items[selected], runHospitalAdmissionScenario);
                break;
            case ACTION_HOSPITAL_INTERACTIVE:
                status = runScenarioOutsideCurses(items[selected], runHospitalAdmissionInteractiveScenario);
                break;
            case ACTION_UNIVERSITY_AUTOMATIC:
                status = runScenarioOutsideCurses(items[selected], runUniversityComplaintScenario);
                break;
            case ACTION_UNIVERSITY_INTERACTIVE:
                status = runScenarioOutsideCurses(items[selected], runUniversityComplaintInteractiveScenario);
                break;
            case ACTION_DOCS:
                status = viewDocumentation(docsLink);
                break;
            case ACTION_EXIT:
                running = false;
                break;
        }
    }

    endwin();
    if (!docsLink.empty()) std::cout << "Documentation: " << docsLink << '\n';
    std::cout << "Exiting TIVIDY demo.\n";
    return 0;
}
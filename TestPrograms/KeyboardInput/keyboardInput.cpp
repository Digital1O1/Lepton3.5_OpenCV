#include <ncurses.h>
#include <string>
#include <cstring> // Required for strlen in the C-style example
// g++ keyboardInput.cpp -o keyboardInput -lncurses && ./keyboardInput
int main()
{
    // --- Ncurses Initialization ---
    initscr(); // Start curses mode
    cbreak();  // Line buffering disabled, pass on every key press
    noecho();  // Don't echo input characters to the screen

    // Get screen dimensions
    int row, col;
    getmaxyx(stdscr, row, col);

    // --- Single Character Input (getch) ---
    mvprintw(0, 0, "Press any key to see its ASCII value (q to quit): ");
    refresh(); // Update the screen

    int ch;
    do
    {
        ch = getch(); // Wait for user input and get the character
        mvprintw(1, 0, "You pressed the key with ASCII value: %d.  ", ch);
        refresh();
    } while (ch != 'q'); // Loop until 'q' is pressed

    // Clear previous output lines
    clrtoeol();
    move(3, 0);
    clrtoeol();

    // --- String Input (getstr) ---
    echo(); // Re-enable echoing for string input so the user can see what they type
    mvprintw(3, 0, "Enter a string: ");
    refresh();

    char str[80]; // Buffer to store the input string
    getstr(str);  // Read a string from the user

    // --- Displaying the Input ---
    noecho(); // Disable echoing again
    mvprintw(4, 0, "You entered: %s", str);
    mvprintw(LINES - 1, 0, "Press any key to exit."); // LINES is a macro set by initscr()
    refresh();

    getch(); // Wait for a final user input before exiting

    // --- Ncurses Shutdown ---
    endwin(); // End curses mode

    return 0;
}

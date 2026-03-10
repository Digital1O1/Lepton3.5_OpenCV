#include <ncurses.h>
#include <string>
#include <vector>
#include <iostream>
// g++ menu.cpp -lncurses -o menu_ncurseExample
int main()
{
    initscr();            // Start curses mode
    clear();              // Clear the screen
    noecho();             // Don't echo input to the screen
    cbreak();             // Disable line buffering, pass keys directly
    keypad(stdscr, TRUE); // Enable the use of arrow keys (KEY_UP/DOWN)
    curs_set(0);          // Hide the cursor

    std::vector<std::string> choices = {"Option 1", "Option 2", "Option 3", "Exit"};
    int highlight = 0;
    int choice = 0;
    int c;

    while (1)
    {
        // Print the menu items
        for (int i = 0; i < choices.size(); ++i)
        {
            if (i == highlight)
            {
                attron(A_REVERSE); // Highlight the current choice
            }
            mvprintw(i + 1, 1, choices[i].c_str()); // Print at (y, x)
            attroff(A_REVERSE);                     // Turn off highlighting
        }

        c = getch(); // Wait for user input
        std::cout << "User input : " << c << std::flush();

        switch (c)
        {
        case KEY_UP:
            if (highlight == 0)
                highlight = choices.size() - 1;
            else
                --highlight;
            break;
        case KEY_DOWN:
            if (highlight == choices.size() - 1)
                highlight = 0;
            else
                ++highlight;
            break;
        case 10: // Enter key has ASCII value 10
            choice = highlight;
            break;
        default:
            break;
        }

        if (choice == choices.size() - 1) // If "Exit" is chosen
            break;
    }

    // Print the final choice before exiting ncurses mode
    mvprintw(choices.size() + 2, 1, "You chose: %s", choices[choice].c_str());
    getch(); // Wait for a key press before closing

    endwin(); // End curses mode

    return 0;
}

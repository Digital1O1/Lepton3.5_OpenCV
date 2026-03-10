#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    // Initialize ncurses
    initscr();            // Start curses mode
    cbreak();             // Line buffering disabled, pass on ctrl characters
    noecho();             // Don't echo input to the screen
    keypad(stdscr, TRUE); // Enable special keys (like arrow keys)

    // Set stdscr to non-blocking mode
    nodelay(stdscr, TRUE); // Make getch() non-blocking, returns ERR if no input

    //
    printw("Press keys (press 'q' to quit). The program loop is running.\n");
    printw("Output will update every 0.5 seconds.\n");

    int ch;
    bool running = true;
    int loop_count = 0;

    while (running)
    {
        // Non-blocking input check
        ch = getch();

        if (ch != ERR)
        {
            // Input received
            if (ch == 'q')
            {
                running = false; // Exit the loop if 'q' is pressed
            }
            else
            {
                // Handle other input
                mvprintw(3, 0, "Key pressed: %c (code: %d)   ", ch, ch);
                // Move cursor to a consistent position and refresh the display of the input
                refresh();
            }
        }

        // Main program logic/task can run continuously here
        mvprintw(4, 0, "Loop count: %d   ", loop_count++);
        refresh(); // Update the screen with the new count

        // Optional: Add a small delay to control loop speed or reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Clean up ncurses
    endwin(); // End curses mode

    return 0;
}

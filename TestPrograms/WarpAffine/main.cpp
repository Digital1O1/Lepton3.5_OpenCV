#include <opencv2/opencv.hpp> // Ncurses defines OK as a preprocessor macro and will cause a bunch of errors if opencv library isn't defined first
// Use the following to kill the macro if ordering can't change
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <thread>
using namespace cv;
using namespace std;
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

    Mat image(400, 400, CV_8UC3, Scalar::all(0));

    // 2. Define circle parameters
    Point center(200, 200);      // Center at (200, 200)
    int radius = 100;            // Radius of 100 pixels
    Scalar color(255, 255, 255); // White color (BGR: blue=255, green=255, red=255)
    int thickness = -1;          // -1 means a filled circle

    // 3. Draw the circle on the image
    circle(image, center, radius, color, thickness);
    string windowName = "Circle Drawing Example";
    namedWindow(windowName, WINDOW_AUTOSIZE);

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
            switch (ch)
            {
            case 258:
                printw("Down\n");
                break;
            case 259:
                printw("Up\n");
                break;
            default:
                printw("Waiting on input...\n");
            }
        }

        // Main program logic/task can run continuously here
        mvprintw(4, 0, "Loop count: %d   ", loop_count++);
        refresh(); // Update the screen with the new count

        imshow(windowName, image);
        cv::waitKey(50);

        // Optional: Add a small delay to control loop speed or reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Clean up ncurses
    endwin(); // End curses mode

    return 0;
}

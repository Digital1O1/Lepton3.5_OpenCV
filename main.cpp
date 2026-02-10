#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::VideoCapture thermalCamera("/dev/video8", cv::CAP_V4L2);
    // cv::VideoCapture visibleCamera(
    //     "libcamerasrc ! video/x-raw,width=640,height=480,format=NV12,framerate=30/1 ! "
    //     "videoconvert ! appsink",
    //     cv::CAP_GSTREAMER);
    /*
    What each piece does (important)
    Element	Why it matters
    queue max-size-buffers=1 leaky=downstream	Drops old frames immediately
    appsink drop=true	Never buffers
    max-buffers=1	Hard cap
    sync=false	Don’t wait for timestamps

    This alone usually drops latency from 500–1500 ms → ~1 frame.
    */
    cv::VideoCapture visibleCamera(
        "libcamerasrc ! "
        "video/x-raw,width=640,height=480,format=BGR,framerate=30/1 ! "
        "queue max-size-buffers=1 leaky=downstream ! "
        "videoconvert ! "
        "appsink drop=true max-buffers=1 sync=false",
        cv::CAP_GSTREAMER);

    // Optional: Set camera parameters (e.g., frame width, height, FPS)

    if (!thermalCamera.isOpened())
    {
        std::cerr << "Failed to open camera\n";
        return -1;
    }

    thermalCamera.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', '1', '6', ' '));
    thermalCamera.set(cv::CAP_PROP_CONVERT_RGB, 0);
    thermalCamera.set(cv::CAP_PROP_FRAME_WIDTH, 160);
    thermalCamera.set(cv::CAP_PROP_FRAME_HEIGHT, 120);

    std::cout << "Lepton 3.5 Radiometric Viewer" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  'q' - Quit" << std::endl;
    std::cout << "  's' - Save frame" << std::endl;
    std::cout << "  'a' - Auto-scale mode" << std::endl;
    std::cout << "  'm' - Manual scale mode (fixed range)" << std::endl;
    std::cout << "  'h' - Flip horizontal" << std::endl;
    std::cout << "  'v' - Flip vertical" << std::endl;
    std::cout << "\n";

    cv::Mat frame, thermal, visible;
    int frame_count = 0;
    bool auto_scale = true;
    bool flip_horizontal = false;
    bool flip_vertical = true;

    // For manual scaling
    double scale_min = 27000; // ~20°C
    double scale_max = 31000; // ~37°C

    while (true)
    {
        thermalCamera >> frame;
        visibleCamera >> visible;
        if (frame.empty() || visible.empty())
            continue;

        frame_count++;

        // Extract thermal data
        if (frame.channels() > 1)
        {
            std::vector<cv::Mat> channels;
            cv::split(frame, channels);
            thermal = channels[0];
        }
        else
        {
            thermal = frame;
        }

        // Flip the thermal image if needed
        if (flip_horizontal && flip_vertical)
        {
            cv::flip(thermal, thermal, -1); // Both axes
        }
        else if (flip_horizontal)
        {
            cv::flip(thermal, thermal, 1); // Horizontal
        }
        else if (flip_vertical)
        {
            cv::flip(thermal, thermal, 0); // Vertical
            cv::flip(visible, visible, 0); // Vertical
        }

        // Get statistics
        double minRaw, maxRaw;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(thermal, &minRaw, &maxRaw, &minLoc, &maxLoc);

        double minT = (minRaw / 100.0) - 273.15;
        double maxT = (maxRaw / 100.0) - 273.15;

        // Get center pixel temperature
        uint16_t centerRaw = thermal.at<uint16_t>(60, 80);
        double centerT = (centerRaw / 100.0) - 273.15;

        // Print every frame for responsiveness
        std::cout << "\rFrame " << frame_count
                  << " | Min: " << std::fixed << std::setprecision(1) << minT << "°C"
                  << " | Max: " << maxT << "°C"
                  << " | Center: " << centerT << "°C"
                  << " | Range: " << (maxT - minT) << "°C     " << std::flush;

        // Create display
        cv::Mat display;

        if (auto_scale)
        {
            // Auto-scale based on current frame
            cv::normalize(thermal, display, 0, 255, cv::NORM_MINMAX);
        }
        else
        {
            // Manual scale for consistent visualization
            thermal.convertTo(display, CV_8U, 255.0 / (scale_max - scale_min),
                              -scale_min * 255.0 / (scale_max - scale_min));
        }

        display.convertTo(display, CV_8U);

        // Apply colormap
        cv::Mat colored;
        cv::applyColorMap(display, colored, cv::COLORMAP_JET);

        // Add overlays
        float fontFace = 0.4;
        std::string mode = auto_scale ? "AUTO" : "MANUAL";
        std::string flip_status = "";
        if (flip_horizontal)
            flip_status += "H";
        if (flip_vertical)
            flip_status += "V";
        if (!flip_status.empty())
            mode += " [" + flip_status + "]";

        cv::putText(colored, mode, cv::Point(5, 15),
                    cv::FONT_HERSHEY_SIMPLEX, fontFace, cv::Scalar(255, 255, 255), 1);
        cv::putText(colored, cv::format("Min: %.1fC", minT), cv::Point(5, 30),
                    cv::FONT_HERSHEY_SIMPLEX, fontFace, cv::Scalar(0, 255, 255), 1);
        cv::putText(colored, cv::format("Max: %.1fC", maxT), cv::Point(5, 45),
                    cv::FONT_HERSHEY_SIMPLEX, fontFace, cv::Scalar(0, 0, 255), 1);
        cv::putText(colored, cv::format("Ctr: %.1fC", centerT), cv::Point(5, 60),
                    cv::FONT_HERSHEY_SIMPLEX, fontFace, cv::Scalar(255, 255, 0), 1);

        // Draw crosshair at center
        cv::line(colored, cv::Point(75, 60), cv::Point(85, 60), cv::Scalar(0, 255, 0), 1);
        cv::line(colored, cv::Point(80, 55), cv::Point(80, 65), cv::Scalar(0, 255, 0), 1);

        // Mark hottest spot
        cv::circle(colored, maxLoc, 3, cv::Scalar(0, 0, 255), 1);

        // Mark coldest spot
        cv::circle(colored, minLoc, 3, cv::Scalar(255, 255, 0), 1);

        // Resize for display
        cv::Mat resized;
        cv::resize(colored, resized, cv::Size(640, 480), 0, 0, cv::INTER_NEAREST);

        cv::imshow("Lepton 3.5 - Thermal", resized);
        cv::imshow("Visibile", visible);

        char key = cv::waitKey(1);
        if (key == 27 || key == 'q')
            break;
        else if (key == 's')
        {
            cv::imwrite(cv::format("thermal_%d.png", frame_count), resized);
            std::cout << "\nSaved frame " << frame_count << std::endl;
        }
        else if (key == 'a')
        {
            auto_scale = true;
            std::cout << "\nAuto-scale mode enabled" << std::endl;
        }
        else if (key == 'm')
        {
            auto_scale = false;
            std::cout << "\nManual scale mode (20-37°C)" << std::endl;
        }
        else if (key == 'h')
        {
            flip_horizontal = !flip_horizontal;
            std::cout << "\nHorizontal flip: " << (flip_horizontal ? "ON" : "OFF") << std::endl;
        }
        else if (key == 'v')
        {
            flip_vertical = !flip_vertical;
            std::cout << "\nVertical flip: " << (flip_vertical ? "ON" : "OFF") << std::endl;
        }
    }

    std::cout << "\nDone!" << std::endl;
    thermalCamera.release();
    cv::destroyAllWindows();
    return 0;
}

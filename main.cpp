#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>

int main()
{
    cv::VideoCapture thermalCamera("/dev/video0", cv::CAP_V4L2);

    /*
    What each piece does (important)
    Element	Why it matters
    queue max-size-buffers=1 leaky=downstream	Drops old frames immediately
    appsink drop=true	Never buffers
    max-buffers=1	Hard cap
    sync=false	Don’t wait for timestamps

    This alone usually drops latency from 500–1500 ms → ~1 thermalFrames.
    */

    // To determine IMX462 camera stuff : rpicam-hello --list-cameras
    cv::VideoCapture visibleCamera(
        "libcamerasrc camera-name=/base/axi/pcie@1000120000/rp1/i2c@88000/imx290@1a ! "
        "video/x-raw,width=640,height=480,format=NV12,framerate=30/1 ! "
        "queue max-size-buffers=1 leaky=downstream ! "
        "videoconvert ! "
        "appsink drop=true max-buffers=1 sync=false",
        cv::CAP_GSTREAMER);

    if (!thermalCamera.isOpened() || !visibleCamera.isOpened())
    {
        std::cerr << "Failed to open camera(s)\n";
        return -1;
    }

    thermalCamera.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', '1', '6', ' '));
    thermalCamera.set(cv::CAP_PROP_CONVERT_RGB, 0);
    thermalCamera.set(cv::CAP_PROP_FRAME_WIDTH, 160);
    thermalCamera.set(cv::CAP_PROP_FRAME_HEIGHT, 120);

    std::cout << "Lepton 3.5 Radiometric Viewer" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  'q' - Quit" << std::endl;
    std::cout << "  's' - Save thermalFrames" << std::endl;
    std::cout << "  'a' - Auto-scale mode" << std::endl;
    std::cout << "  'm' - Manual scale mode (fixed range)" << std::endl;
    std::cout << "  'h' - Flip horizontal" << std::endl;
    std::cout << "  'v' - Flip vertical" << std::endl;
    std::cout << "\n";

    cv::Mat thermalFrames, thermal, visible;
    cv::Mat edges, gray;
    cv::Mat thermalAndVisible;

    int frame_count = 0;
    bool auto_scale = true;
    bool flip_horizontal = false;
    bool flip_vertical = true;

    // Parameters for Canny
    int lowerThreshold = 100;
    int upperThreshold = 200;
    int apertureSize = 3;
    bool L2gradientStatus = false;

    // For manual scaling
    double scale_min = 27000; // ~20°C
    double scale_max = 31000; // ~37°C

    while (true)
    {
        thermalCamera >> thermalFrames;
        visibleCamera >> visible;

        if (thermalFrames.empty() || visible.empty())
            continue;

        frame_count++;

        // Extract thermal data
        if (thermalFrames.channels() > 1)
        {
            std::vector<cv::Mat> channels;
            cv::split(thermalFrames, channels);
            thermal = channels[0];
        }
        else
        {
            thermal = thermalFrames;
        }

        // Flip the thermal image if needed
        if (flip_horizontal && flip_vertical)
        {
            cv::flip(thermal, thermal, -1);
        }
        else if (flip_horizontal)
        {
            cv::flip(thermal, thermal, 1);
        }
        else if (flip_vertical)
        {
            cv::flip(thermal, thermal, 0);
            cv::flip(visible, visible, 0);
        }

        // ----------- Visible Edge Detection -----------

        cv::cvtColor(visible, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, lowerThreshold, upperThreshold, apertureSize, L2gradientStatus);

        // ----------- Thermal Statistics -----------

        double minRaw, maxRaw;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(thermal, &minRaw, &maxRaw, &minLoc, &maxLoc);

        double minT = (minRaw / 100.0) - 273.15;
        double maxT = (maxRaw / 100.0) - 273.15;

        uint16_t centerRaw = thermal.at<uint16_t>(60, 80);
        double centerT = (centerRaw / 100.0) - 273.15;

        std::cout << "\rFrame " << frame_count
                  << " | Min: " << std::fixed << std::setprecision(1) << minT << "°C"
                  << " | Max: " << maxT << "°C"
                  << " | Center: " << centerT << "°C"
                  << " | Range: " << (maxT - minT) << "°C     " << std::flush;

        // ----------- Thermal Display Creation -----------

        cv::Mat thermalDisplay;

        if (auto_scale)
        {
            cv::normalize(thermal, thermalDisplay, 0, 255, cv::NORM_MINMAX);
        }
        else
        {
            thermal.convertTo(thermalDisplay, CV_8U,
                              255.0 / (scale_max - scale_min),
                              -scale_min * 255.0 / (scale_max - scale_min));
        }

        thermalDisplay.convertTo(thermalDisplay, CV_8U);

        cv::Mat colored;
        cv::applyColorMap(thermalDisplay, colored, cv::COLORMAP_JET);

        // Resize thermal to match visible
        cv::Mat thermalResized;
        cv::resize(colored, thermalResized, visible.size(), 0, 0, cv::INTER_NEAREST);

        // ----------- Edge Overlay (Correct Way) -----------

        cv::Mat overlay = visible.clone();

        cv::Mat mask;
        cv::threshold(edges, mask, 0, 255, cv::THRESH_BINARY);

        cv::Mat redOverlay(visible.size(), CV_8UC3, cv::Scalar(0, 0, 255));
        redOverlay.copyTo(overlay, mask);

        // ----------- Thermal Fusion (Optional Layering) -----------

        cv::Mat fusion;
        double thermalAlpha = 0.4;
        cv::addWeighted(overlay, 1.0, thermalResized, thermalAlpha, 0.0, fusion);

        // ----------- Display -----------

        cv::hconcat(overlay, fusion, thermalAndVisible);

        cv::imshow("Visible | Thermal Fusion", thermalAndVisible);
        cv::imshow("Canny Edge Detection", edges);

        char key = cv::waitKey(1);
        if (key == 27 || key == 'q')
            break;
        else if (key == 's')
        {
            cv::imwrite(cv::format("thermal_%d.png", frame_count), thermalResized);
            std::cout << "\nSaved thermalFrames " << frame_count << std::endl;
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
    visibleCamera.release();
    cv::destroyAllWindows();
    return 0;
}
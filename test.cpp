#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera\n";
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 160);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 120);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','1','6',' '));

    cv::Mat raw16;
    while (true) {
        cap >> raw16;

        if (raw16.empty()) continue;

        // Convert raw -> Celsius
        cv::Mat tempC = raw16.clone();
        tempC.convertTo(tempC, CV_32F);
        tempC = (tempC / 100.0f) - 273.15f;

        double minT, maxT;
        cv::minMaxLoc(tempC, &minT, &maxT);
        std::cout << "Min: " << minT << " C | Max: " << maxT << " C\n";

        // Visualization
        cv::Mat display;
        cv::normalize(raw16, display, 0, 255, cv::NORM_MINMAX);
        display.convertTo(display, CV_8U);
        cv::applyColorMap(display, display, cv::COLORMAP_INFERNO);
	cv::resize(display,display,cv::Size(640,480));
        cv::imshow("Thermal", display);
        if (cv::waitKey(1) == 27) break;
    }
}


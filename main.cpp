#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera\n";
        return -1;
    }
    
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','1','6',' '));
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 160);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 120);
    
    std::cout << "Lepton 3.5 Radiometric Viewer" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  'q' - Quit" << std::endl;
    std::cout << "  's' - Save frame" << std::endl;
    std::cout << "  'a' - Auto-scale mode" << std::endl;
    std::cout << "  'm' - Manual scale mode (fixed range)" << std::endl;
    std::cout << "\n";
    
    cv::Mat frame, thermal;
    int frame_count = 0;
    bool auto_scale = true;
    
    // For manual scaling
    double scale_min = 27000;  // ~20°C
    double scale_max = 31000;  // ~37°C
    
    while (true) {
        cap >> frame;
        
        if (frame.empty()) continue;
        
        frame_count++;
        
        // Extract thermal data
        if (frame.channels() > 1) {
            std::vector<cv::Mat> channels;
            cv::split(frame, channels);
            thermal = channels[0];
        } else {
            thermal = frame;
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
	// The std::flush is what's causing the 'single line' on the terminal
        
        // Create display
        cv::Mat display;
        
        if (auto_scale) {
            // Auto-scale based on current frame
            cv::normalize(thermal, display, 0, 255, cv::NORM_MINMAX);
        } else {
            // Manual scale for consistent visualization
	    // min = 27000 | max = 31000
            thermal.convertTo(display, CV_8U, 255.0 / (scale_max - scale_min), 
                            -scale_min * 255.0 / (scale_max - scale_min));
        }
        // This is needed to use applyColorMap() 
        display.convertTo(display, CV_8U);
        
        // Apply colormap
        cv::Mat colored;
        cv::applyColorMap(display, colored, cv::COLORMAP_JET);
        
        // Add overlays
	float fontFace = 0.4;
        std::string mode = auto_scale ? "AUTO" : "MANUAL";
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
        
        char key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
        else if (key == 's') {
            cv::imwrite(cv::format("thermal_%d.png", frame_count), resized);
            std::cout << "\nSaved frame " << frame_count << std::endl;
        }
        else if (key == 'a') {
            auto_scale = true;
            std::cout << "\nAuto-scale mode enabled" << std::endl;
        }
        else if (key == 'm') {
            auto_scale = false;
            std::cout << "\nManual scale mode (20-37°C)" << std::endl;
        }
    }
    
    std::cout << "\nDone!" << std::endl;
    cap.release();
    cv::destroyAllWindows();
    return 0;
}

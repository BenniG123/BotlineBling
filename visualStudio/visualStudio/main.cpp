#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
	cv::VideoCapture cap;
	cv::Mat frame;
	cv::Mat dFrame;
	const int thresh = 120;
	uchar colorCnt[5] = { 0 };
	cv::Scalar colors[5] = { cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0), cv::Scalar(0, 128, 255) };
	uchar activeNote;
	cv::namedWindow("Window", cv::WINDOW_AUTOSIZE);
	/*cv::namedWindow("Threshold", cv::WINDOW_AUTOSIZE);
	cv::createTrackbar("thresh", "Threshold", &thresh, 255);*/

	cap.open(argv[1]);
	if (!cap.isOpened()) {
		std::cout << "Couldn't open video file\n";
		return -1;
	}
	cap.set(CV_CAP_PROP_POS_FRAMES, 60);

	while (1) {
		cap >> frame;
		activeNote = 0;
		if (frame.empty()) {
			break;
		}
		cv::cvtColor(frame, frame, CV_BGR2GRAY);
		frame = frame(cv::Rect(350, 240, 600, 480));
		cv::resize(frame, frame, cv::Size(frame.cols / 2, frame.rows / 2));
		cv::threshold(frame, frame, thresh, 255, cv::THRESH_BINARY);
		cv::dilate(frame, dFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

		cv::cvtColor(dFrame, dFrame, CV_GRAY2BGR);

		for (int i = 0; i < 5; i++) {
			cv::Mat tmp = dFrame(cv::Rect(65+40*i, 80, 10, 10));
			if (tmp.at<cv::Vec3b>(4, 4) == cv::Vec3b::all(255)) {
				colorCnt[i]++;

				if (colorCnt[i] >= 3) {
					activeNote = 1;
				}
			}
			else {
				colorCnt[i] = 0;
			}
		}

		if (activeNote) {
			for (int i = 0; i < 5; i++) {
				// Check the other notes if they're 255
				if (colorCnt[i] > 0) {
					cv::circle(dFrame, cv::Point(65 + 40 * i, 85), 10, colors[i], 3);
				}
			}
		}

		cv::imshow("Window", dFrame); // Show our image inside it.

		char c = cv::waitKey(60);
		if (c == 27)
			break;
	}

	return 0;
}
/*
Primary
Verify that the scheduler works
Write the velocity/prediction code
Write the serial/message code

Secondary
Make sure everything works at 640 better
Sustained notes
Monitor star power
Test with solo sections (blue bg)

*/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <queue>

struct NoteEvent {
	int index;
	int frame;
	
	NoteEvent(int index, int frame) {
		this->index = index;
		this->frame = frame;
	}
};

class Scheduler {
private:
	std::queue<NoteEvent> q;
	const int LOOKAHEAD = 3;

public:
	void addEvent(NoteEvent e) {
		q.push(e);
	}

	std::vector<NoteEvent> nextNotes(int curFrame) {
		// Find events that need to happen next
		std::vector<NoteEvent> ret;
		for (int i = 0; i < q.size(); i++) {
			// If q.front is within the next 
			if (q.front().frame <= curFrame + LOOKAHEAD) {
				// Put in vector
				ret.push_back(q.front());
				q.pop();
			}
			else {
				break;
			}
		}
	}
};

int main(int argc, char** argv) {
	cv::VideoCapture cap;
	cv::Mat frame;
	cv::Mat dFrame;
	
	uchar colorCnt[5] = { 0 };
	cv::Scalar colors[5] = { cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0), cv::Scalar(0, 128, 255) };
	
	// Threshold of the grayscale input image
	int THRESH = 120;
    int frameCounter = 0;
    Scheduler scheduler;

	const int MAXUCHAR = 255;

	// Region of interest where the notes are
	/*const int ROIX = 350;
	const int ROIY = 240;
	const int ROIWIDTH = 600;
	const int ROIHEIGHT = 480;*/

	// Note distribution
	const int TOPNOTEOFFSETX = 79;
	const int TOPNOTEOFFSETY = 60;
	const int TOPNOTESPACINGX = 35;
	const int BOTNOTEOFFSETX = 59;
	const int BOTNOTEOFFSETY = 114;
	const int BOTNOTESPACINGX = 44;

	const int CONSEC_HITS = 1;

	cv::namedWindow("Window", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("Window2", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("Threshold", cv::WINDOW_NORMAL);
	cv::resizeWindow("Threshold", 300, 20);
	cv::createTrackbar("thresh", "Threshold", &THRESH, 255);

	if (strcmp(argv[1], "cam") == 0) {
		cap.open(1);
	}
	else {
		cap.open(argv[1]);
		//cap.set(CV_CAP_PROP_POS_FRAMES, 400);
	}
	if (!cap.isOpened()) {
		std::cout << "Couldn't open video file\n";
		return -1;
	}
	
	while (1) {
        frameCounter++;
		cap >> frame;
		if (frame.empty()) {
			break;
		}
		cv::cvtColor(frame, frame, CV_BGR2GRAY);
		//frame = frame(cv::Rect(ROIX, ROIY, ROIWIDTH, ROIHEIGHT));
		cv::resize(frame, frame, cv::Size(frame.cols / 2, frame.rows / 2));
		cv::Mat frame2;
		frame.copyTo(frame2);
		cv::threshold(frame, frame, THRESH, MAXUCHAR, cv::THRESH_BINARY);
		cv::dilate(frame, dFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

		cv::cvtColor(dFrame, dFrame, CV_GRAY2BGR);
		cv::cvtColor(frame2, frame2, CV_GRAY2BGR);

		for (int i = 0; i < 5; i++) {
			cv::Mat tmp = dFrame(cv::Rect((TOPNOTEOFFSETX - 5) + TOPNOTESPACINGX * i, (TOPNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10));
			cv::rectangle(frame2, cv::Rect((TOPNOTEOFFSETX - 5) + TOPNOTESPACINGX * i, (TOPNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			cv::rectangle(dFrame, cv::Rect((TOPNOTEOFFSETX - 5) + TOPNOTESPACINGX * i, (TOPNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			if (tmp.at<cv::Vec3b>(4, 4) == cv::Vec3b::all(MAXUCHAR)) {
				colorCnt[i]++;

				if (colorCnt[i] >= CONSEC_HITS) {
                    // TODO - Store top frame number in a temp queue
					cv::circle(frame2, cv::Point(TOPNOTEOFFSETX + TOPNOTESPACINGX * i, TOPNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
					cv::circle(dFrame, cv::Point(TOPNOTEOFFSETX + TOPNOTESPACINGX * i, TOPNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
				}
			}
			else {
				colorCnt[i] = 0;
			}
		}

		for (int i = 0; i < 5; i++) {
			cv::Mat tmp = dFrame(cv::Rect((BOTNOTEOFFSETX - 5) + BOTNOTESPACINGX * i, (BOTNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10));
			cv::rectangle(frame2, cv::Rect((BOTNOTEOFFSETX - 5) + BOTNOTESPACINGX * i, (BOTNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			cv::rectangle(dFrame, cv::Rect((BOTNOTEOFFSETX - 5) + BOTNOTESPACINGX * i, (BOTNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			if (tmp.at<cv::Vec3b>(4, 4) == cv::Vec3b::all(MAXUCHAR)) {
				colorCnt[i]++;

				if (colorCnt[i] >= CONSEC_HITS) {
                    // TODO - Pop off of top queue and calculate frame difference for speed
                    int speed = 5;
                    scheduler.addEvent(NoteEvent(frameCounter + speed, i));
					cv::circle(frame2, cv::Point(BOTNOTEOFFSETX + BOTNOTESPACINGX * i, BOTNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
					cv::circle(dFrame, cv::Point(BOTNOTEOFFSETX + BOTNOTESPACINGX * i, BOTNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
				}
			}
			else {
				colorCnt[i] = 0;
			}
		}

		cv::imshow("Window", frame2);
		cv::imshow("Window2", dFrame);

		char c = cv::waitKey(60);
		if (c == 27)
			break;
	}

	return 0;
}

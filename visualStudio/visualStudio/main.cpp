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
#include <Windows.h>

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
	const int LOOKAHEAD = 2;
	
public:
	void addEvent(NoteEvent e) {
		q.push(e);
	}

	uchar nextNotes(int curFrame) {
		// Find events that need to happen next
		uchar ret = 0;
		
		if (!q.empty() && q.front().frame <= curFrame) {
			while (!q.empty() && q.front().frame <= curFrame + LOOKAHEAD) {
				// Construct uchar - Say which buttons to push in
				ret |= 1 << q.front().index;
				// Strum - Bit 5
				ret |= 0x20;
				q.pop();
			}
		}

		return ret;
	}
};

int main(int argc, char** argv) {
	cv::VideoCapture cap;
	cv::Mat frame;
	cv::Mat dFrame;
	float avgSpeed = 0;
	int noteCounter = 0;

	uchar colorCntTop[5] = { 0 };
	uchar colorCntBot[5] = { 0 };
	cv::Scalar colors[5] = { cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0), cv::Scalar(0, 128, 255) };
	std::string colorNames[5] = { "Green", "Red", "Yellow", "Blue", "Orange" };

	std::queue<int> noteQ[5];

	HANDLE m_hCommPort = ::CreateFile("COM3", GENERIC_WRITE | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	
	// Threshold of the grayscale input image
	int THRESH = 168;
    int frameCounter = 0;
    Scheduler scheduler;
	uchar toSend;
	uchar prevSent;

	const int MAXUCHAR = 255;
	const int DIST_TO_FIRE = 36;

	// Region of interest where the notes are
	const int ROIX = 20;
	const int ROIY = 0;
	const int ROIWIDTH = 600;
	const int ROIHEIGHT = 480;

	// Note distribution
	const int TOPNOTEOFFSETX = 93;
	const int TOPNOTEOFFSETY = 80;
	const int TOPNOTESPACINGX = 28;
	const int BOTNOTEOFFSETX = 76;
	const int BOTNOTEOFFSETY = 134;
	const int BOTNOTESPACINGX = 36;

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
		cap.set(CV_CAP_PROP_POS_MSEC, 3000);
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
		frame = frame(cv::Rect(ROIX, ROIY, ROIWIDTH, ROIHEIGHT));
		cv::resize(frame, frame, cv::Size(frame.cols / 2, frame.rows / 2));
		cv::Mat frame2;
		frame.copyTo(frame2);

		cv::threshold(frame, frame, THRESH, MAXUCHAR, cv::THRESH_BINARY);
		cv::dilate(frame, dFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

		cv::cvtColor(dFrame, dFrame, CV_GRAY2BGR);
		cv::cvtColor(frame2, frame2, CV_GRAY2BGR);

		// Draw guides
		cv::line(frame2, cv::Point(96, 35), cv::Point(20, 240), cv::Scalar(0, 0, 255), 1);
		cv::line(frame2, cv::Point(200, 35), cv::Point(276, 240), cv::Scalar(0, 0, 255), 1);
		cv::circle(frame2, cv::Point(147, 216), 14, cv::Scalar(0, 0, 255), 1);

		for (int i = 0; i < 5; i++) {
			cv::Mat tmp = dFrame(cv::Rect((TOPNOTEOFFSETX - 5) + TOPNOTESPACINGX * i, (TOPNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10));
			cv::rectangle(frame2, cv::Rect((TOPNOTEOFFSETX - 5) + TOPNOTESPACINGX * i, (TOPNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			cv::rectangle(dFrame, cv::Rect((TOPNOTEOFFSETX - 5) + TOPNOTESPACINGX * i, (TOPNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			if (tmp.at<cv::Vec3b>(4, 4) == cv::Vec3b::all(MAXUCHAR)) {
				colorCntTop[i]++;

				if (colorCntTop[i] >= CONSEC_HITS) {
                    // Store top frame number in a queue
					if (colorCntTop[i] == CONSEC_HITS) {
						noteQ[i].push(frameCounter);
					}

					// Draw circles where we found stuff
					cv::circle(frame2, cv::Point(TOPNOTEOFFSETX + TOPNOTESPACINGX * i, TOPNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
					cv::circle(dFrame, cv::Point(TOPNOTEOFFSETX + TOPNOTESPACINGX * i, TOPNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
				}
			}
			else {
				colorCntTop[i] = 0;
			}
		}

		for (int i = 0; i < 5; i++) {
			cv::Mat tmp = dFrame(cv::Rect((BOTNOTEOFFSETX - 5) + BOTNOTESPACINGX * i, (BOTNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10));
			cv::rectangle(frame2, cv::Rect((BOTNOTEOFFSETX - 5) + BOTNOTESPACINGX * i, (BOTNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			cv::rectangle(dFrame, cv::Rect((BOTNOTEOFFSETX - 5) + BOTNOTESPACINGX * i, (BOTNOTEOFFSETY - 5) + (abs(2 - i) * abs(2 - i)), 10, 10), colors[i]);
			if (tmp.at<cv::Vec3b>(4, 4) == cv::Vec3b::all(MAXUCHAR)) {
				colorCntBot[i]++;

				if (colorCntBot[i] >= CONSEC_HITS) {
					// Pop off of top queue and calculate frame difference for speed
					if (!noteQ[i].empty() && colorCntBot[i] == CONSEC_HITS) {
						// Calculate pixels/frame
						float speed = ((float)(BOTNOTEOFFSETY - TOPNOTEOFFSETY) / (frameCounter - noteQ[i].front()));
						noteQ[i].pop();

						if (noteCounter <= 20) {
							avgSpeed = (avgSpeed * noteCounter + speed) / (noteCounter + 1);
							noteCounter++;
						}

						// Calculate frames to note impact
						scheduler.addEvent(NoteEvent(i, frameCounter + (int) (DIST_TO_FIRE / speed)));
					}

					cv::circle(frame2, cv::Point(BOTNOTEOFFSETX + BOTNOTESPACINGX * i, BOTNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
					cv::circle(dFrame, cv::Point(BOTNOTEOFFSETX + BOTNOTESPACINGX * i, BOTNOTEOFFSETY + (abs(2 - i) * abs(2 - i))), 10, colors[i], 3);
				}
			}
			else if (!noteQ[i].empty() && noteCounter >= 20 && noteQ[i].front() < (frameCounter - DIST_TO_FIRE * 2 / avgSpeed)) {
				// Check to make sure we don't have phantom notes in the queue
				noteQ[i].pop();
				std::cout << "Popped missed note from queue\n";
			}
			else {
				colorCntBot[i] = 0;
			}
		}

		cv::imshow("Window", frame2);
		cv::imshow("Window2", dFrame);

		toSend = scheduler.nextNotes(frameCounter);

		//std::cout << noteQ[0].size() << " | " << noteQ[1].size() << " | " << noteQ[2].size() << " | " << noteQ[3].size() << " | " << noteQ[4].size() << "\r";

		// Send toSend to Arduino
		if (toSend != 0) {
			std::cout << "0x" << std::hex << (int)toSend;
			std::cout << "\n";
		
			DWORD bytesWritten;
			while (!WriteFile(m_hCommPort, &toSend, 1, &bytesWritten, NULL));
			toSend = 0;
			while (!WriteFile(m_hCommPort, &toSend, 1, &bytesWritten, NULL));
		}

		char c = cv::waitKey(30);
		if (c == 27)
			break;
	}

	return 0;
}

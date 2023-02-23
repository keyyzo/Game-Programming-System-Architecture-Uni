#pragma once
#include <chrono>

/*
	This class and its functions are
	a close adaptation of github user
	mcleary, real name Thales Sabino,
	timer class using std::chrono

	Source: https://gist.github.com/mcleary/b0bf4fa88830ff7c882d

*/

typedef std::chrono::steady_clock timer_clock;

class Timer
{
public:

	// timer functions

	void Start();
	void Stop();
	void GetTimeStop();
	double elapsedMS();
	double elapsedSeconds();

	

private:

	// timer variables

	bool isTimerRunning;
	timer_clock::time_point startTime;
	timer_clock::time_point endTime;
	timer_clock::time_point healthTimerStart;
	timer_clock::time_point healthTimerEnd;


};


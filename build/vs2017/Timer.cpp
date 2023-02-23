#include "Timer.h"

/*
	This class and its functions are
	a close adaptation of github user
	mcleary, real name Thales Sabino,
	timer class using std::chrono

	Source: https://gist.github.com/mcleary/b0bf4fa88830ff7c882d

*/

// starts the timer
void Timer::Start()
{
	startTime = std::chrono::steady_clock::now();
	isTimerRunning = true;
}

// stops and resets the timer
void Timer::Stop()
{
	endTime = std::chrono::steady_clock::now();
	startTime = std::chrono::steady_clock::now();
	isTimerRunning = false;
}

// stops the timer
void Timer::GetTimeStop()
{
	endTime = std::chrono::steady_clock::now();
	isTimerRunning = false;
}

// returns time in milliseconds
double Timer::elapsedMS()
{
	timer_clock::time_point finalTime;

	// checks if timer is running
	// returns a currently running time
	if (isTimerRunning)
	{
		finalTime = timer_clock::now();
	}

	// if not
	// returns a final time
	else
	{
		finalTime = endTime;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - startTime).count();

	
}

// returns time in seconds

double Timer::elapsedSeconds()
{
	return elapsedMS() / 1000.0;
}

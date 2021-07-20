#pragma once


class Timer
{
public:

	Timer();
	~Timer();

	float get_delta_time();

private:

	float old_time = 0.0f;
	float cur_time = 0.0f;

};
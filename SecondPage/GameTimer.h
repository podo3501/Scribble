//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#pragma once

struct PassConstants;

class CGameTimer
{
public:
	CGameTimer();
	~CGameTimer();

	CGameTimer(const CGameTimer&) = delete;
	CGameTimer& operator=(const CGameTimer&) = delete;

	float TotalTime() const; // in seconds
	float DeltaTime() const; // in seconds

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

	void GetPassCB(PassConstants* outPc);

private:
	double mSecondsPerCount{ 0.0 };
	double mDeltaTime{ -1.0 };

	__int64 mBaseTime{ 0 };
	__int64 mPausedTime{ 0 };
	__int64 mStopTime{ 0 };
	__int64 mPrevTime{ 0 };
	__int64 mCurrTime{ 0 };

	bool mStopped{ false };
};
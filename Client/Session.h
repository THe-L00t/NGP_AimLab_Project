#pragma once
// 화면(씬)이 게임 상태를 받는 창구
// 씬은 게임 로직을 직접 읽지 않고, 이 인터페이스의 snapshot()으로 상태를 받는다. 

struct Snapshot;

class Session
{
public:
	virtual ~Session() = default;

	virtual const Snapshot& snapshot() const = 0;
	virtual void update(float) = 0;
};


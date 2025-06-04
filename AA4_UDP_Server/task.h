#pragma once
class Task {
public: 
	virtual void Execute() = 0;
	virtual ~Task() = default;
};
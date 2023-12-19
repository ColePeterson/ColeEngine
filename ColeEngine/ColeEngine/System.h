#pragma once


class Engine;

class System
{
public:
	System() = default;
	virtual ~System() = default;

	virtual void update(Engine & engine) = 0;

};
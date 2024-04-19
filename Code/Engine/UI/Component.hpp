#pragma once

class Component {
public:
	Component() {};
	virtual ~Component() {};
	
	virtual void Update() = 0;
	virtual void Render() const = 0;
};
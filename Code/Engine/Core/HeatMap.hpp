#pragma once
#include <vector>
#include "Engine/Math/IntVec2.hpp"

class HeatMap
{
public:
	HeatMap(IntVec2 const& dimensions);
	void SetAllValues(float value);
	float GetValueOfTile(const IntVec2& coords) const;
	void SetValueOfTile(const IntVec2& coords, float valueToSet);
	void AddValueToTile(const IntVec2& coords, float valueToAdd);
	float GetHighestValue() const;

protected:
	std::vector<float> m_values;
	IntVec2 m_dimensions;
	float m_highestValue = 0.0f;
};
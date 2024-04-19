#include "Engine/Core/HeatMap.hpp"

HeatMap::HeatMap(IntVec2 const& dimensions):m_dimensions(dimensions)
{
	m_values.resize(dimensions.x * dimensions.y);
	for (int i = 0; i < m_values.size() ; i++) {
		m_values[i] = 0.0f;
	}
}

void HeatMap::SetAllValues(float value)
{
	for (int i = 0; i < m_values.size() ; i++) {
		m_values[i] = value;
	}
}

float HeatMap::GetValueOfTile(const IntVec2& coords) const
{
	return m_values[coords.y * m_dimensions.x + coords.x];
}

void HeatMap::SetValueOfTile(const IntVec2& coords, float valueToSet)
{
	if (valueToSet > m_highestValue)
		m_highestValue = valueToSet;
	m_values[coords.y * m_dimensions.x + coords.x] = valueToSet;
}

void HeatMap::AddValueToTile(const IntVec2& coords, float valueToAdd)
{
	float valueToSet = m_values[coords.y * m_dimensions.x + coords.x] + valueToAdd;
	if (valueToSet > m_highestValue)
		m_highestValue = valueToSet;
	m_values[coords.y * m_dimensions.x + coords.x] = valueToSet;
}

float HeatMap::GetHighestValue() const
{
	return m_highestValue;
}

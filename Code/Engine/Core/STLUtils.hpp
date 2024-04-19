#pragma once
#include <vector>

template<typename T>
void ClearAndDeleteEverything(std::vector<T*>& myVector)
{
	for (int index = 0; index < (int)myVector.size(); index++) {
		delete myVector[index];	//Deleting a nullptr is ok in c++
	}
	myVector.clear();
}

#include <iostream>

#include "Mogre.hpp"

void Update(float frameTime)
{
	float pos[3];
	float qRot[4];
	Mogre::GameEntity* entity = Mogre::CreateGameEntity("TestMesh.glb", pos, qRot);
	std::cout << frameTime << " " << entity << "\n";
}

int main()
{
	std::cout << "Hello, Ogre!\n";

	Mogre::ThreadData* threadData = Mogre::Initialize();
	Mogre::SetUpdateCallback(Update);
	Mogre::Run(threadData);

	return 0;
}

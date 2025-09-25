#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <memory>

class GameEntity
{
	Transform transform;
	std::shared_ptr<Mesh> mesh;

public:
	GameEntity(std::shared_ptr<Mesh> mesh);
	~GameEntity();
	
	// Getters
	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform(); // return reference to avoid writing directly to the transform

	void Draw();
};


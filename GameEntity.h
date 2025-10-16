#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
#include <memory>

class GameEntity
{
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~GameEntity();
	
	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	Transform& GetTransform(); // return reference to avoid writing directly to the transform

	// Setters
	void SetMaterial(std::shared_ptr<Material> material);

	void Draw();
};


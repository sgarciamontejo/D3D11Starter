#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;
	transform = Transform();
}

GameEntity::~GameEntity()
{
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> GameEntity::GetMaterial() {
	return material;
}

Transform& GameEntity::GetTransform()
{
	return transform;
}

void GameEntity::SetMaterial(std::shared_ptr<Material> material) {
	this->material = material;
}

void GameEntity::Draw() {
	mesh->Draw();
}
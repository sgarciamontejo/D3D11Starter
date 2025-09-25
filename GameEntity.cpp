#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
	transform = Transform();
}

GameEntity::~GameEntity()
{
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return std::shared_ptr<Mesh>();
}

Transform& GameEntity::GetTransform()
{
	return transform;
}

void GameEntity::Draw() {
	mesh->Draw();
}
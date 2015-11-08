#include "Alien.h"


Alien::Alien(btDiscreteDynamicsWorld* worldN,Terrain* terrianN,Camera* cameraN)
{
	translate = glm::vec3(0, 0, 0);
	rotate = glm::vec3(1.0f, 0.0f, 0.0f);
	terrain = terrianN;
	world = worldN;
	camera = cameraN;
	prevAngle = 0;
	prevVeloc = 0;
	fragmentName = "fragment-shader[none].txt";
	float height = 5.0f*METER;
	GetVertices().push_back({ { -height / 4.0f, height, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
	GetVertices().push_back({ { height / 4.0f, height, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
	GetVertices().push_back({ { -height / 4.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } });
	GetVertices().push_back({ { height / 4.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } });

	GetIndices().push_back({ glm::uvec3(2, 0, 1)});
	GetIndices().push_back({ glm::uvec3(2, 1, 3) });

	textureName = "alien.png";


	shape = new btBoxShape(btVector3(0.1f*METER, 0.5f*METER, 0.1f*METER));

	Load(); //loads drawing related stuff. Call after vertices/indices have been defined
}
void Alien::Update(double){

}
glm::mat4 billboard(glm::vec3 position, glm::vec3 cameraPos, glm::vec3 cameraUp) {
	glm::vec3 look = normalize(cameraPos - position);
	glm::vec3 right = cross(cameraUp, look);
	glm::vec3 up2 = cross(look, right);
	glm::mat4 transform;
	transform[0] = glm::vec4(right, 0);
	transform[1] = glm::vec4(glm::vec3(0,1,0), 0);
	transform[2] = glm::vec4(look, 0);
	// Uncomment this line to translate the position as well
	// (without it, it's just a rotation)
	//transform[3] = vec4(position, 0);
	return transform;
}
void Alien::UpdatePosition(){
	translate = alienMovement(translate, camera->position());
	position = glm::translate(glm::mat4(),glm::vec3(translate.x,translate.y,translate.z));

	glm::mat4 rotat = billboard(glm::vec3(translate.x, 0.0f, translate.z), glm::vec3(camera->position().x, 0.0f, camera->position().z), camera->up());

	//glm::vec2 ang1 = glm::normalize(glm::vec2(rotate.x, rotate.z));
	//glm::vec2 ang2 = glm::normalize(glm::vec2(camera->position().x - translate.x, camera->position().z-translate.z));
	//float angle = glm::atan(ang2.y - ang1.y, ang2.x - ang1.x)*glm::atan(ang2.y - ang1.y, ang2.x - ang1.x);
	position = position*rotat;


}
Alien::~Alien()
{
}


glm::vec3 Alien::alienMovement(glm::vec3 alien, glm::vec3 player) {

	float radius = 0.4*KILOMETER;
	glm::vec3 center(0, 0, 0);
	std::normal_distribution<float> velDistro(1, 2);
	float velocity = (1 / glm::length(alien - player)) * METER*25;
	float angle = 0;

	//If alien is far away:
	glm::vec2 alien2D(alien.x, alien.z);
	if (abs(radius - length(alien2D)) < (radius/8)) {
		float currentLoc = atan2(alien.z, alien.x);
		if (currentLoc < 0) {
			currentLoc = 360 + currentLoc;
		}
		std::normal_distribution<float> distro3(currentLoc - 30, currentLoc + 30);
		angle = GetDistribution(distro3);
		if (angle < 0) {
			angle = 360 + angle;
		}
		else if (angle > 360) {
			angle = angle - 360;
		}
	}

	else if (glm::length(alien - player) > (radius/4)) {
		std::normal_distribution<float> distro2(prevAngle-30.0, prevAngle+30.0);
		angle = GetDistribution(distro2);
		if (angle < 0) {
			angle = 360 + angle;
		}
		else if (angle > 360) {
			angle = angle - 360;
		}
	}
	else {
		float playerLoc = atan2(player.z, player.x);
		playerLoc = playerLoc*-1;
		if (playerLoc < 0) {
			playerLoc = 360 + playerLoc;
		}
		std::normal_distribution<float> distro3(playerLoc - 45, playerLoc + 45);
		angle = GetDistribution(distro3);
		if (angle < 0) {
			angle = 360 + angle;
		}
		else if (angle > 360) {
			angle = angle - 360;
		}
	}

	//If alien is closer, always move away: 
	/*else {
		if (randomNumber(0, distance from alien to wall) > a value) {
			float angle = randomNumber(angles away from player);
		}
		else {
			float angle = randomNumber(angles away from wall and player)
		}

	}*/

	if ((rand() % 100) < 98) {
		angle = prevAngle;
		velocity = prevVeloc;
	}

	prevVeloc = velocity;
	prevAngle = angle;
	alien.x = alien.x + (velocity * cos(angle));
	alien.z = alien.z + (velocity * sin(angle));
	alien.y = terrain->GetHeight(alien.x, alien.z);

	return alien;

}


//put global includes in 'BasicIncludes'
//
#include "BasicIncludes.h"
#include "rand.h"
#include "Camera.h"
#include "Input.h"
#include "Object.h"
#include "Terrain.h"
#include "Skybox.h"
#include "Alien.h"
#include "Planet.h"
#include "Rock.h"

//Function List
void Update(double);
void GetPositions();
void DecrementTimers();
void Draw();
void CameraInput();
void MouseInput();
void InitializeWindow();
void Terminate();
void Run();

//Variables
GLFWwindow* mainThread;
glm::uvec2 SCREEN_SIZE;
Camera camera = Camera();
glm::vec2 mouseChangeDegrees;
double deltaTime;
double physicsTimer;
bool runPhysics;
double timeMod;
int seed;
bool wireframeToggle;
double wireframeTimer;
std::vector<Object*> objects;

Terrain* terrain;
irrklang::ISoundEngine* engine;
irrklang::ISound* music;
void Terminate() {
	glfwTerminate();
	exit(0);
}

void TogglePhysics(){
	if (physicsTimer <= 0){
		runPhysics = !runPhysics;
		physicsTimer = 0.35f;
	}
}
void ToggleWireFrame(){
	if (wireframeTimer <= 0){
		
		if (wireframeToggle){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else if (!wireframeToggle){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		}

		wireframeToggle = !wireframeToggle;
		wireframeTimer = 0.35f;
	}
}
void InitializeWindow() {
	engine = irrklang::createIrrKlangDevice();
	engine->setSoundVolume(1.0f);
	music = engine->play3D("Alien Jive.mp3",
		irrklang::vec3df(0, 0, 0), true, false, true);
	if (music){
	
		music->setMinDistance(1.0f*KILOMETER);
		music->setPosition(irrklang::vec3df(0, 0, 0));
		music->setVolume(0.35f);
	}

	wireframeToggle = false;
	seed = time(NULL);
	runPhysics = false;
	physicsTimer = 0;
	wireframeTimer = 0;

	if (!glfwInit()) {
		Terminate();
	}

	//set screen size
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCREEN_SIZE = glm::uvec2(1280,720);

	//basic aa done for us ;D
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);


	//can change the screen setting later


	//if (window == FULLSCREEN) {

	//	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//	mainThread = glfwCreateWindow(SCREEN_SIZE.x, SCREEN_SIZE.y, "LifeSim", glfwGetPrimaryMonitor(), NULL);

	//}
	//else if (window == WINDOWED) {

		//glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		//mainThread = glfwCreateWindow(SCREEN_SIZE.x, SCREEN_SIZE.y, "Space Trip", NULL, NULL);

	//}
	//else if (BORDERLESS) {

	//	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		//const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		mainThread = glfwCreateWindow(mode->width, mode->height, "LifeSim", glfwGetPrimaryMonitor(), NULL);

	//}


	if (!mainThread) {
		glfwTerminate();
		throw std::runtime_error("GLFW window failed");
	}

	glfwMakeContextCurrent(mainThread);

	// initialise GLEW
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("glewInit failed");
	}

	glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/

	glfwSetInputMode(mainThread, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(mainThread, SCREEN_SIZE.x / 2.0, SCREEN_SIZE.y / 2.0);

	//Discard all the errors
	while (glGetError() != GL_NO_ERROR) {}

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthMask(GL_TRUE);  // turn on
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mouseChangeDegrees = glm::vec2(0);

	// setup camera 
	camera.setViewportAspectRatio(SCREEN_SIZE.x / (float)SCREEN_SIZE.y);

	camera.setPosition(glm::vec3(-METER*25.0f, METER*25.0f, METER*25.0f));
	camera.offsetOrientation(45.0f, 10.0f);

	//unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
	//threads = new ThreadPool(concurentThreadsSupported);


	//for keyboard controls
	glfwSetKeyCallback(mainThread, InputKeyboardCallback);
	SetInputWindow(mainThread);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	timeMod *= (1.0f+(yoffset / 15.0));
	if (timeMod<0.005){
		timeMod = 0.005;
	}
}

void Run() {
	

		SetKey(GLFW_KEY_ESCAPE, std::bind(&Terminate));
		SetKey(GLFW_KEY_SPACE, std::bind(&TogglePhysics));
		SetKey(GLFW_KEY_E, std::bind(&ToggleWireFrame));

		deltaTime = 1.0 / 60.0;
		InitializeWindow();

		glfwSetScrollCallback(mainThread, ScrollCallback);

		//Init values and objects

		// Build the broadphase
		btBroadphaseInterface* broadphase = new btDbvtBroadphase();

		// Set up the collision configuration and dispatcher
		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

		// The actual physics solver
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		// The world.
		btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		world->setGravity(btVector3(0, -9.82*METER, 0));


		Skybox* hand = new Skybox(world);
		Object* handP = hand;
		objects.push_back(handP);

		terrain = new Terrain(world, KILOMETER,500,seed);
		Object* terrainP = terrain;
		objects.push_back(terrainP);
		for (int i = 1; i < 5; i++){
			Rock* rock = new Rock(world, terrain);
			Object* rockP = rock;
			objects.push_back(rockP);
		}

		Alien* man = new Alien(world, terrain, &camera, engine, 0);
		Object* manP = man;
		objects.push_back(manP);

		for (int i = 0; i < 50;i++){
			int r = i % 14+1;
			Alien* alien = new Alien(world, terrain, &camera, engine, r);
			Object* alienP = alien;
			objects.push_back(alienP);
		}


		std::vector<glm::vec3> planetVecs;
		std::uniform_int_distribution<int> numDistro(10, 20);
		int numPlanets = GetDistribution(numDistro);

		for (int i = 0; i < numPlanets; i++) {
			glm::vec3 newVec;
			bool ok = false;
			int check = 0;
			while (!ok) {
				check++;
				if (check > 100) {
					std::cout << "planet not placed\n";
					break;
				}
				newVec = glm::vec3((rand() % 2000 + 500) * METER * (-1 + 2*((rand() % 100)<50)), 
					(rand() % 2000 + 500) * METER * (-1 + 2*((rand() % 100)<50)), (rand() % 2000 + 500) * METER * (-1 + 2*((rand() % 100)<50)));
				ok = true;
				for (int j = 0; j < planetVecs.size(); j++) {
					if ((glm::length(newVec - planetVecs[j]) < (160 * METER))
						|| (abs(glm::length(planetVecs[j] - glm::vec3(0, 0, 0)) - glm::length(newVec - glm::vec3(0, 0, 0)))) < (160 * METER)){
						ok = false;
					}
				}
			}
			planetVecs.push_back(newVec);
			Planet* planet = new Planet(world, newVec);
			Object* planetP = planet;
			objects.push_back(planetP);
		}


		//timer info for loop
		double t = 0.0f;
		double currentTime = glfwGetTime();
		double accumulator = 0.0f;

		glfwPollEvents();		//stop loop when glfw exit is called
		glfwSetCursorPos(mainThread, SCREEN_SIZE.x / 2.0f, SCREEN_SIZE.y / 2.0f);


		//THIS IS TO SPEED UP TIME
		timeMod = 1.0f;


		while (!glfwWindowShouldClose(mainThread)) {
			double newTime = glfwGetTime();
			double frameTime = newTime - currentTime;
			//std::cout << "FPS:: " <<1.0f / frameTime << std::endl;

			//setting up timers
			if (frameTime > 0.25) {
				frameTime = 0.25;
			}
			currentTime = newTime;
			accumulator += frameTime;

			//# of updates based on accumulated time

			while (accumulator >= deltaTime) {

				
				engine->setListenerPosition(irrklang::vec3df(camera.position().x, camera.position().y, camera.position().x), irrklang::vec3df(camera.forward().x, camera.forward().y, camera.forward().x));
	
				MouseInput();//update mouse change
				glfwPollEvents(); //executes all set input callbacks

				CameraInput(); //bypasses input system for direct camera manipulation
				//if (runPhysics){	
					Update(deltaTime*timeMod); //updates all objects based on the constant deltaTime.
					world->stepSimulation(deltaTime*timeMod, glm::max(10 * timeMod,10.0));
			//	}
				GetPositions(); //transforms bullet matrices to opengl


				DecrementTimers();

				t += deltaTime;
				accumulator -= deltaTime;
			}


			//draw
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			Draw();
			glfwSwapBuffers(mainThread);
	}

		music->drop(); // release music stream.

		engine->drop(); // delete engine

	//cleanup
		delete world;
		delete solver;
		delete dispatcher;
		delete collisionConfiguration;
		delete broadphase;

}

void DecrementTimers(){
	if (physicsTimer>0){
		physicsTimer = physicsTimer - deltaTime;
		wireframeTimer = wireframeTimer - deltaTime;
	}
	if (wireframeTimer>0){
		wireframeTimer = wireframeTimer - deltaTime;
	}
}
void MouseInput() {
	double xPos;
	double yPos;
	glfwGetCursorPos(mainThread, &xPos, &yPos);
	xPos -= (SCREEN_SIZE.x / 2.0);
	yPos -= (SCREEN_SIZE.y / 2.0);

	mouseChangeDegrees.x = (float)(xPos / SCREEN_SIZE.x *camera.fieldOfView().x);
	mouseChangeDegrees.y = (float)(yPos / SCREEN_SIZE.y *camera.fieldOfView().y);

	/*std::cout << "Change in x (mouse): " << mouseChangeDegrees.x << std::endl;
	std::cout << "Change in y (mouse): " << mouseChangeDegrees.y << std::endl;*/

	camera.offsetOrientation(mouseChangeDegrees.x, mouseChangeDegrees.y);

	glfwSetCursorPos(mainThread, SCREEN_SIZE.x / 2.0f, SCREEN_SIZE.y / 2.0f);
}
void CameraInput() {
	double moveSpeed;
	glm::vec3 oldPos=camera.position();
		moveSpeed = 50 * METER * deltaTime;



	if (glfwGetKey(mainThread, GLFW_KEY_S) == GLFW_PRESS) {
		camera.offsetPosition(float(moveSpeed) * -camera.forward());
	}
	else if (glfwGetKey(mainThread, GLFW_KEY_W) == GLFW_PRESS) {
		camera.offsetPosition(float(moveSpeed) * camera.forward());
	}
	if (glfwGetKey(mainThread, GLFW_KEY_A) == GLFW_PRESS) {
		camera.offsetPosition(float(moveSpeed) * -camera.right());
	}
	else if (glfwGetKey(mainThread, GLFW_KEY_D) == GLFW_PRESS) {
		camera.offsetPosition(float(moveSpeed) * camera.right());
	}
	if (glfwGetKey(mainThread, GLFW_KEY_Z) == GLFW_PRESS) {
		camera.offsetPosition(float(moveSpeed) * -glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(mainThread, GLFW_KEY_X) == GLFW_PRESS) {
		camera.offsetPosition(float(moveSpeed) * glm::vec3(0, 1, 0));
	}

	if (glm::distance(glm::vec3(0), camera.position())>CUTOFF){
     	camera._position = oldPos;
	}

	camera._position.y = terrain->GetHeight(camera._position.x, camera._position.z) + 5 * METER;
}
void GetPositions(){
	for (int i = 0; i < objects.size(); i++){
		objects[i]->UpdatePosition();
	}
}
void Update(double dt) {
	for (int i = 0; i < objects.size(); i++){
		objects[i]->Update(dt);
	}
}
void Draw() {
	for (int i = 0; i < objects.size();i++){
		objects[i]->Draw(camera);
	}
}
int main(){

	Run();

	Terminate();

	return 0;
}

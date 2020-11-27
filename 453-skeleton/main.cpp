#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setNormals(cpuGeom.normals);
	gpuGeom.setTextCoordinates(cpuGeom.textCoords);
	gpuGeom.setIndices(cpuGeom.indices);
}

class Scenery
{
public:
	Scenery(
		std::string const & texturePath,
		float const scale,
		glm::vec3 const & pos,
		float const rotationInPlaceSpeed,
		float const orbitRotationStep,
		float const axialTilt,
		float const orbitalInclination,
		Scenery* relativeScene = nullptr
	)
		: gpu_()
		, texture_(texturePath, GL_NEAREST)
		, scale_(scale)
		, pos_(glm::rotate(pos, glm::radians(orbitalInclination), glm::vec3(0, 0, 1)))
		, rotationOnAxisStep_(rotationInPlaceSpeed)
		, orbitRotationStep_(orbitRotationStep)
		, axialTilt_(axialTilt)
		, orbitalInclination_(orbitalInclination)
		, relativeScene_(relativeScene)
		{}

	void reset()
	{
		rotationOnAxisAngle_ = 0.0f;
		orbitRotationAngle_ = 0.0f;
	}

	glm::mat4 getModel() const
	{
		glm::mat4 M(1.0f);
		M = glm::scale(M, glm::vec3(scale_, scale_, scale_)); //Scale

		M = glm::rotate(M, glm::radians(rotationOnAxisAngle_), glm::rotate(glm::vec3(0,1,0), glm::radians(orbitalInclination_-axialTilt_), glm::vec3(0,0,1))); //Rotate on axis
		M = glm::rotate(M,  glm::radians(orbitalInclination_-axialTilt_), glm::vec3(0, 0, 1)); //Tilt

		return getTranslation() * M; //Translation and rotation in orbit.
	}

	void update(const float secondsElapsed)
	{
		rotationOnAxisAngle_ += secondsElapsed * rotationOnAxisStep_;
		orbitRotationAngle_ += secondsElapsed * orbitRotationStep_;
		if (rotationOnAxisAngle_ > 360) rotationOnAxisAngle_ -= 360;
		if (orbitRotationAngle_ > 360) orbitRotationAngle_ -= 360;
	}

	void render()
	{
		gpu_.bind();
		texture_.bind();
		glDrawElements(GL_TRIANGLES, GLsizei(gpu_.getIndexSize()), GL_UNSIGNED_INT, 0);
		texture_.unbind();
	}

	GPU_Geometry gpu_;
	Texture texture_;
private:

	glm::mat4 getTranslation() const
	{
		glm::mat4 T(1.0f); 

		if (relativeScene_ != nullptr)
		{
			T *= relativeScene_->getTranslation();
		}
		glm::vec3 orbitalInclinationAxis = glm::rotate(
			glm::vec3(0, 1, 0),
			glm::radians(orbitalInclination_),
			glm::vec3(0, 0, 1)
		);
		glm::vec3 rotatedPos = glm::rotate(pos_, glm::radians(orbitRotationAngle_), orbitalInclinationAxis);
		T *= glm::translate(rotatedPos);
		return T;
	}

	const float scale_;
	const glm::vec3 pos_;
	const float rotationOnAxisStep_;
	const float orbitRotationStep_;
	const float axialTilt_;
	const float orbitalInclination_;
	const Scenery* relativeScene_;

	float rotationOnAxisAngle_ = 0.0f;
	float orbitRotationAngle_ = 0.0f;
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 0.0, 2.0), aspect(1.0f) {
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_P && action == GLFW_PRESS) //pause
		{
			pauseDown = !pauseDown;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) //pause
		{
			reset = true;
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp, Scenery& const scenery) {
		glm::mat4 M = scenery.getModel();
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "light");
		glm::vec3 light = camera.getPos();
		glUniform3fv(location, 1, glm::value_ptr(light));

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	bool getReset()
	{
		if (reset)
		{
			reset = false;
			return true;
		}
		return false;
	}

	bool pauseDown = false;
	Camera camera;
private:
	bool rightMouseDown = false;
	bool reset = false;
	float aspect;
	double mouseOldX;
	double mouseOldY;

};

std::vector<unsigned int> createTriangleMesh(
	const std::vector<std::vector<glm::vec3>>& matrixSurface
)
{
	std::vector<unsigned int> indices;
	for (unsigned int i = 1; i < matrixSurface.size(); i++)
	{
		for (unsigned int j = 1; j < matrixSurface[i].size(); j++)
		{
			indices.push_back(i * (unsigned int)matrixSurface[i].size() + j);
			indices.push_back((i - 1) * (unsigned int)matrixSurface[i].size() + j);
			indices.push_back((i - 1) * (unsigned int)matrixSurface[i].size() + j - 1);

			indices.push_back((i - 1) * (unsigned int)matrixSurface[i].size() + j - 1);
			indices.push_back(i * (unsigned int)matrixSurface[i].size() + j - 1);
			indices.push_back(i * (unsigned int)matrixSurface[i].size() + j);
		}
	}
	return indices;
}

void createSphere(CPU_Geometry& cpuGeom)
{
	constexpr float radius = 1;
	std::vector<std::vector<glm::vec3>> matrixSphere;
	for (float phi = 0; phi < 180.001 ; phi += 10)
	{
		std::vector<glm::vec3> rowVerts;
		for (float theta = 0; theta <= 360.001; theta += 20)
		{
			glm::vec3 point = {
				radius * std::cos(glm::radians(theta)) * std::sin(glm::radians(phi)),
				radius * cos(glm::radians(phi)),
				radius * std::sin(glm::radians(theta)) * std::sin(glm::radians(phi)),
			};
			rowVerts.push_back(point);
			cpuGeom.verts.insert(cpuGeom.verts.begin(), point);
			cpuGeom.normals.insert(cpuGeom.normals.begin(), point);
			cpuGeom.textCoords.emplace_back(
				theta/360.0f,
				phi/180.0f
			);
		}
		matrixSphere.push_back(std::move(rowVerts));
	}
	cpuGeom.indices = createTriangleMesh(matrixSphere);
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	// The current CPU_Geometry and GPU_Geometry classes are defined in
	// Geometry.h/Geometry.cpp They will work for this assignment, but for some of
	// the bonuses you may have to modify them.
	CPU_Geometry sphere;
	createSphere(sphere);

	Scenery stars("images/stars.jpg", 10.0f, glm::vec3(0.0, 0.0, 0.0), 0.0f, 0, 0, 0);
	Scenery sun("images/sun.jpg", 0.25f, glm::vec3(0.0,0.0,0.0), 360.0f/45.0f, 0, 0, 0);
	Scenery earth("images/earth.jpg", 0.1f, glm::vec3(0.8, 0.0, 0), 360.0f, 360.0f/80.f, 30.f, 20.f, &sun);
	Scenery moon("images/moon.jpg", 0.05f, glm::vec3(0.3, 0.0, 0.0), 360.0f/5.0f, 360.0f/10.0f, 0, -30.f, &earth);
	
	updateGPUGeometry(earth.gpu_, sphere);
	updateGPUGeometry(moon.gpu_, sphere);
	updateGPUGeometry(sun.gpu_, sphere);
	for (auto& normal : sphere.normals)
	{
		normal *= -1;
	}
	updateGPUGeometry(stars.gpu_, sphere);


	using clock = std::chrono::system_clock;

	auto beforeTime = clock::now();
	// RENDER LOOP
	while (!window.shouldClose()) {
		std::chrono::duration<float> elapsedTime = clock::now() - beforeTime;
		beforeTime = clock::now();
		glfwPollEvents();

		if (a4->pauseDown)
		{
			elapsedTime = std::chrono::duration<float>::zero();
		}
		if (a4->getReset())
		{
			moon.reset();
			sun.reset();
			stars.reset();
			earth.reset();
		}
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();

		a4->viewPipeline(shader, stars);
		stars.render();

		sun.update(elapsedTime.count());
		a4->viewPipeline(shader, sun);
		sun.render();

		earth.update(elapsedTime.count());
		a4->viewPipeline(shader, earth);
		earth.render();

		moon.update(elapsedTime.count());
		a4->viewPipeline(shader, moon);
		moon.render();


		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}

#include <algorithm>

#include "../nclgl/window.h"
#include "../nclgl/GameTimer.h"
#include "Renderer.h"

const float pi = std::acos(-1);

int main() {
	
	Window w("Vertex Transformation!", 800, 600, false);
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);
	if (!renderer.HasInitialised()) {
		return -1;
	}

	float scale = 100.0f;
	float rotation = 0.0f;
	Vector3 position(0, 0, -1500.0f);
	float fov = 0.25;
	GameTimer* gameTime = w.GetTimer();
	float heldTime = 0.0f;

	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1)) {
			renderer.SwitchToOrthographic();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {
			renderer.SwitchToPerspective(); 
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS))  ++scale;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS)) --scale;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))  ++rotation;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT)) --rotation;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_K))
			position.y -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_I))
			position.y += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_J))
			position.x -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_L))
			position.x += 1.0f;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_O))
			position.z -= 1.0f;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_P))
			position.z += 1.0f;

		// Uncomment to oscillate fov between 0.25 and 0.75
		//fov = 0.5f * static_cast<float>(abs(sin(gameTime->GetTotalTimeSeconds()))) + 0.25f;

		// Uncomment for aim down (no damping) 
		/*if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT)) {
			heldTime += gameTime->GetTimeDeltaSeconds();
			heldTime = std::min(heldTime, 0.5f);
			fov = (0.5f * std::sin((pi) * heldTime)) + 0.25f;
		}
		else {
			fov = 0.25f;
			heldTime = 0.0f;
		}*/
		

		renderer.SetRotation(rotation);
		renderer.SetScale(scale);
		renderer.SetPosition(position);
		renderer.SetFOV(fov);

		renderer.UpdateScene(gameTime->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
	}

	return 0;
}
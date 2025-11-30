#include "../NCLGL/window.h"
#include "Renderer.h"

int main()	{
	Window w("Make your own project!", 1280, 720, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);
	w.GetTimer()->Tick();

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		renderer.SetTime(w.GetTimer()->GetTotalTimeSeconds());

		float dt = std::min(w.GetTimer()->GetTimeDeltaSeconds(), 1 / 30.0f);// min FPS: 30 fps
		renderer.UpdateScene(dt);

		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
	}
	return 0;
}
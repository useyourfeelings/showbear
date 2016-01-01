#include <glutils.h>
#include <thirdparty/glfw/include/GLFW/glfw3.h>
#include <thirdparty/imgui/imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <cstdlib>

#include <iostream>
using namespace std;

#include <model.h>
#include <glslprogram.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

int width = 800;
int height = 600;
GLFWwindow *window = nullptr;
Model *man = nullptr;
Model *torus = nullptr;
Model *plane = nullptr;
float lastTime;
float angle;
int rotating = 1;

glm::mat4 model, view, projection;

glm::vec3 eye, look, up;
glm::vec3 light_eye, light_look, light_up;

GLSLProgram prog;

void CompileAndLinkShader()
{
	try
	{
    	prog.compileShader("data/shaders/stenciltest.vs");
    	prog.compileShader("data/shaders/stenciltest.fs");
    	prog.link();
    	prog.validate();
    	prog.use();
    }
    catch(GLSLProgramException & e) {
 		cerr<<e.what()<<endl;
 		exit(EXIT_FAILURE);
    }
}

int Clear()
{
    delete man;
    delete torus;
    delete plane;

    return 0;
}

int InitScene()
{
    CompileAndLinkShader();
    glEnable(GL_DEPTH_TEST);


    man = new Model("data/models/man.dae");
    torus = new Model("data/models/torus.dae");
    plane = new Model("data/models/plane.dae");

    prog.setUniform("light.intensity", vec3(0.85f));

    eye = vec3(8.0f, 6.0f, 10.0f);
    look = vec3(0.0f, 0.0f, 0.0f);
    up = vec3(0.0f, 1.0f, 0.0f);

    light_eye = vec3(-6.0f, 20.0f, 10.0f);
    light_look = vec3(0.0f,0.0f,0.0f);
    light_up = vec3(0.0f,1.0f,0.0f);

    glClearStencil(0);

    return 0;
}

void SetMatrices()
{
    mat4 mv = view * model;
    prog.setUniform("mv", mv);
    prog.setUniform("normal_transform", mat3(glm::transpose(glm::inverse(mv))));
    prog.setUniform("mvp", projection * mv);
    prog.setUniform("light.position", mv * vec4(light_eye, 1.0f));
}

int DrawScene()
{
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);

#if 1
    glStencilFunc(GL_GREATER, 0x20, 0xFFFF);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    //glStencilOpSeparate(GL_FRONT, GL_INCR, GL_INCR, GL_INCR);

    model = mat4(1.0f);
    model = glm::rotate(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)) * model;
    SetMatrices();
    prog.setUniform("material.kd", 0.4f, 0.2f, 0.7f);
    prog.setUniform("material.ka", 0.4f, 0.3f, 0.6f);
    prog.setUniform("material.ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("material.shininess", 15.0f);
    man->Render();
#endif

#if 1
    //glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_EQUAL, 0x1, 0xFFFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    //glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);

    model = glm::translate(vec3(6.f, 3.f, 0.f));
    model = glm::rotate(glm::radians(-angle), glm::vec3(0.f, 1.f, 0.f)) * model;
    SetMatrices();
    prog.setUniform("material.kd", 0.9f, 0.2f, 0.2f);
    prog.setUniform("material.ka", 0.7f, 0.2f, 0.2f);
    prog.setUniform("material.ks", 0.8f, 0.8f, 0.8f);
    prog.setUniform("material.shininess", 5.0f);
    torus->Render();
#endif

#if 1
    glStencilFunc(GL_NOTEQUAL, 0x1, 0xFFFF);

    model = glm::translate(vec3(0.f, -10.f, 0.f));
    SetMatrices();
    prog.setUniform("material.kd", 0.3f, 0.6f, 0.3f);
    prog.setUniform("material.ka", 0.3f, 0.6f, 0.3f);
    prog.setUniform("material.ks", 0.2f, 0.2f, 0.2f);
    prog.setUniform("material.shininess", 55.0f);
    plane->Render();
#endif

    return 0;
}

int Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    view = glm::lookAt(eye, look, up);
    DrawScene();

    return 0;
}

int Update(float time)
{
    float delta_t;
    float delta_angle;
    if(lastTime == 0)
    {
        lastTime = time;
    }

    delta_t = time - lastTime;
    lastTime = time;

    if(rotating)
    {
        delta_angle = delta_t * 90;
        angle += delta_angle;
    }

    return 0;
}

int Resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;

    projection = glm::perspective(glm::radians(90.0f), (float)width / height, 1.f, 400.0f);

    return 0;
}

int DoImgui()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10,10));
    if(!ImGui::Begin("stencil test", NULL, ImVec2(0,0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::End();
    }
    ImGui::Text("space : toggle rotation\nesc : exit");
    ImGui::End();

    ImGui::Render();

    if(ImGui::IsKeyPressed(GLFW_KEY_SPACE))
        rotating = 1 - rotating;
    if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GL_TRUE);

    return 0;
}

///////////////////////////
///////////////////////////
///////////////////////////

void MainLoop()
{
	while(!glfwWindowShouldClose(window))
    {
		GLUtils::checkForOpenGLError(__FILE__,__LINE__);

		// render scene
		Update(float(glfwGetTime()));
		Render();

		// imgui
        DoImgui();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void ResizeGL(int w, int h)
{
    Resize(w,h);
}

void InitializeGL()
{
    glClearColor(0.3f,0.3f,0.3f,1.0f);
    glDebugMessageCallback(GLUtils::debugCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
		GL_DEBUG_SEVERITY_NOTIFICATION, -1 , "Start debugging");

    InitScene();
}

void error_callback(int error, const char* description)
{
    cout<<"Error: "<<description<<endl;
}

int main(int argc, char *argv[])
{
    glfwSetErrorCallback(error_callback);

	// Initialize GLFW
	if(!glfwInit()) exit(EXIT_FAILURE);

	// Select OpenGL 4.3 with a forward compatible core profile.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// Open the window
	window = glfwCreateWindow( width, height, "stencil test", NULL, NULL );
	if(!window)
    {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	ImGui_ImplGlfwGL3_Init(window, true);

	// Load the OpenGL functions.
	cout<<"gladLoadGL() return "<<gladLoadGL()<<endl;

	GLUtils::dumpGLInfo();

	// Initialization
	InitializeGL();
	ResizeGL(width, height);

	// Enter the main loop
	MainLoop();

	ImGui_ImplGlfwGL3_Shutdown();

	// Close window and terminate GLFW
	glfwTerminate();

	Clear();
	// Exit program
	exit(EXIT_SUCCESS);
}

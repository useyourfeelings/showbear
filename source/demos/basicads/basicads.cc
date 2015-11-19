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
    	prog.compileShader("../source/data/shaders/basicads.vs");
    	prog.compileShader("../source/data/shaders/basicads.fs");
    	prog.link();
    	prog.validate();
    	prog.use();
    }
    catch(GLSLProgramException & e)
    {
 		cerr<<e.what()<<endl;
 		exit(EXIT_FAILURE);
    }
}

int Clear()
{
    delete man;
    delete torus;
    delete plane;
}

int InitScene()
{
    CompileAndLinkShader();
    glEnable(GL_DEPTH_TEST);

    man = new Model("../source/data/models/man.dae");
    torus = new Model("../source/data/models/torus.dae");
    plane = new Model("../source/data/models/plane.dae");

    prog.setUniform("light.intensity", vec3(0.85f));

    // eye parameters
    eye = vec3(8.0f, 6.0f, 10.0f);
    look = vec3(0.0f, 0.0f, 0.0f);
    up = vec3(0.0f, 1.0f, 0.0f);

    // light parameters
    light_eye = vec3(-6.0f, 20.0f, 10.0f);
    light_look = vec3(0.0f,0.0f,0.0f);
    light_up = vec3(0.0f,1.0f,0.0f);

    prog.setUniform("light.position", glm::lookAt(light_eye, light_look, light_up) * vec4(light_eye, 1.0f));
}

void SetMatrices()
{
    // set shader parameters
    mat4 mv = view * model;
    prog.setUniform("mv", mv);
    prog.setUniform("normal_transform", mat3(glm::transpose(glm::inverse(mv)))); // transform a normal vector
    prog.setUniform("mvp", projection * mv);
}

int DrawScene()
{
    // draw man
    model = mat4(1.0f);
    model = glm::rotate(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)) * model;
    SetMatrices();
    prog.setUniform("material.kd", 0.4f, 0.2f, 0.7f);
    prog.setUniform("material.ka", 0.4f, 0.3f, 0.6f);
    prog.setUniform("material.ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("material.shininess", 15.0f);
    man->Render();

    // draw torus
    model = glm::translate(vec3(6.f, 3.f, 0.f));
    model = glm::rotate(glm::radians(-angle), glm::vec3(0.f, 1.f, 0.f)) * model;
    SetMatrices();
    prog.setUniform("material.kd", 0.9f, 0.2f, 0.2f);
    prog.setUniform("material.ka", 0.7f, 0.2f, 0.2f);
    prog.setUniform("material.ks", 0.8f, 0.8f, 0.8f);
    prog.setUniform("material.shininess", 5.0f);
    torus->Render();

    // draw plane
    model = glm::translate(vec3(0.f, -8.f, 0.f));
    SetMatrices();
    prog.setUniform("material.kd", 0.3f, 0.6f, 0.3f);
    prog.setUniform("material.ka", 0.3f, 0.6f, 0.3f);
    prog.setUniform("material.ks", 0.2f, 0.2f, 0.2f);
    prog.setUniform("material.shininess", 55.0f);
    plane->Render();
}

int Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    view = glm::lookAt(eye, look, up);
    DrawScene();
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
        delta_angle = delta_t * 60;
        angle += delta_angle;
    }
}

int Resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;

    projection = glm::perspective(glm::radians(90.0f), (float)width / height, 1.f, 400.0f);
}

int DoImgui()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10,10));
    if(!ImGui::Begin("basic ads", NULL, ImVec2(0,0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
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
	window = glfwCreateWindow(width, height, "basicads", NULL, NULL);
	if(!window)
    {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//glfwSetKeyCallback(window, key_callback);
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

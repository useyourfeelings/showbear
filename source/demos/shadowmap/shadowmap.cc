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
int shadowmap_width = 800;
int shadowmap_height = 600;

GLuint shadow_fbo;

GLFWwindow *window = nullptr;
Model *man = nullptr;
Model *torus = nullptr;
Model *plane = nullptr;
float lastTime;
float angle;
int rotating = 1;
GLuint pcf = 0;
GLuint depth_texture;

glm::mat4 model, view, projection, shadow_bias;

glm::vec3 eye, look, up;
glm::vec3 light_eye, light_look, light_up;

GLSLProgram prog;
GLuint render_nothing_routine, render_with_shadow_routine;

void CompileAndLinkShader()
{
	try
	{
    	prog.compileShader("../source/data/shaders/shadowmap.vs");
    	prog.compileShader("../source/data/shaders/shadowmap.fs");
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

int SetupFBO()
{
    glGenTextures(1, &depth_texture);
    cout<<"depth_texture = "<<depth_texture<<endl;
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowmap_width, shadowmap_height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    glGenFramebuffers(1, &shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

    GLenum draw_buffers[] = {GL_NONE};
    glDrawBuffers(1, draw_buffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(result == GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer is complete.\n");
    else
        printf("Framebuffer is not complete.\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int InitScene()
{
    CompileAndLinkShader();
    glEnable(GL_DEPTH_TEST);

    man = new Model("../source/data/models/man.dae");
    torus = new Model("../source/data/models/torus.dae");
    plane = new Model("../source/data/models/plane.dae");

    prog.setUniform("light.intensity", vec3(0.85f));
    prog.setUniform("pcf", pcf);

    eye = vec3(5.0f, 6.0f, 12.0f);
    look = vec3(0.0f, 0.0f, 0.0f);
    up = vec3(0.0f, 1.0f, 0.0f);

    light_eye = vec3(-6.0f, 20.0f, 10.0f);
    light_look = vec3(0.0f,0.0f,0.0f);
    light_up = vec3(0.0f,1.0f,0.0f);

    shadow_bias = mat4( vec4(0.5f,0.0f,0.0f,0.0f),
                        vec4(0.0f,0.5f,0.0f,0.0f),
                        vec4(0.0f,0.0f,0.5f,0.0f),
                        vec4(0.5f,0.5f,0.5f,1.0f));

    SetupFBO();

    GLuint programHandle = prog.getHandle();
    render_nothing_routine = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "RenderNothing");
    render_with_shadow_routine = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "RenderWithShadow");
}

void SetMatrices()
{
    mat4 mv = view * model;
    prog.setUniform("mv", mv);
    prog.setUniform("normal_transform", mat3(glm::transpose(glm::inverse(mv))));
    prog.setUniform("mvp", projection * mv);
    prog.setUniform("mvp_light", shadow_bias * projection * glm::lookAt(light_eye, light_look, light_up) * model);
}

int DrawScene()
{
    model = glm::rotate(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f));
    SetMatrices();
    prog.setUniform("material.kd", 0.4f, 0.2f, 0.7f);
    prog.setUniform("material.ka", 0.4f, 0.3f, 0.6f);
    prog.setUniform("material.ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("material.shininess", 10.0f);
    man->Render();

    model = glm::rotate(glm::radians(-angle), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(vec3(6.f, 6.f, 0.f));
    SetMatrices();
    prog.setUniform("material.kd", 0.9f, 0.2f, 0.2f);
    prog.setUniform("material.ka", 0.7f, 0.2f, 0.2f);
    prog.setUniform("material.ks", 0.8f, 0.8f, 0.8f);
    prog.setUniform("material.shininess", 5.0f);
    torus->Render();

    model = glm::translate(vec3(0.f, -10.f, 0.f));
    SetMatrices();
    prog.setUniform("material.kd", 0.3f, 0.6f, 0.3f);
    prog.setUniform("material.ka", 0.3f, 0.6f, 0.2f);
    prog.setUniform("material.ks", 0.2f, 0.2f, 0.2f);
    prog.setUniform("material.shininess", 55.0f);
    plane->Render();
}

int Pass1()
{
    // use shadow_fbo. just render to depth buffer(depth_texture) in the light's point of view.
    glEnable(GL_DEPTH_TEST);
    view = glm::lookAt(light_eye, light_look, light_up);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowmap_width, shadowmap_height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &render_nothing_routine);
    DrawScene();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int Pass2()
{
    // render normally. test vertex depth against depth_texture in shader.
    view = glm::lookAt(eye, look, up);
    prog.setUniform("light.position", view * vec4(light_eye, 1.0f));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &render_with_shadow_routine);
    glEnable(GL_POLYGON_OFFSET_FILL); // z-fight
    glPolygonOffset(2.0f, 2.0f);
    DrawScene();
}

int Render()
{
    Pass1();
    Pass2();
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

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    if(!ImGui::Begin("shadowmap", NULL, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::End();
    }
    ImGui::Text("space : toggle rotation\nesc : exit\np : toggle simple PCF(percentage-closer filtering)");
    ImGui::End();

    ImGui::Render();

    if(ImGui::IsKeyPressed(GLFW_KEY_SPACE))
        rotating = 1 - rotating;
    if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GL_TRUE);
    if(ImGui::IsKeyPressed(GLFW_KEY_P))
    {
        pcf = 1 - pcf;
        prog.setUniform("pcf", pcf);
    }
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
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1 , "Start debugging");

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
	if(!glfwInit())
        exit(EXIT_FAILURE);

	// Select OpenGL 4.3 with a forward compatible core profile.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// Open the window
	window = glfwCreateWindow(width, height, "shadowmap", NULL, NULL);
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

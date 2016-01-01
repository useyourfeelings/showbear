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

GLuint color_depth_fbo, dummy_vao;
GLuint diffuse_specular_texture;
GLFWwindow *window = nullptr;
Model *vase = nullptr;
Model *torus = nullptr;
Model *plane = nullptr;
Model *cube = nullptr;
Model *sphere = nullptr;
float lastTime;
float angle;
int rotating = 1;
int eye_position = 0;
int show_volume = 0;

glm::mat4 model, view, projection;

glm::vec3 eye, look, up;
glm::vec3 light_eye, light_look, light_up;

GLSLProgram scene_prog, volume_prog, shadow_prog;

void CompileAndLinkShader()
{
	try
	{
	    // for pass1
    	scene_prog.compileShader("data/shaders/sv_scene.vs");
    	scene_prog.compileShader("data/shaders/sv_scene.fs");
    	scene_prog.link();
    	scene_prog.validate();

    	// for pass2
    	volume_prog.compileShader("data/shaders/sv_volume.vs");
    	volume_prog.compileShader("data/shaders/sv_volume.fs");
    	volume_prog.compileShader("data/shaders/sv_volume.gs");
    	volume_prog.link();
    	volume_prog.validate();

    	// for pass3
    	shadow_prog.compileShader("data/shaders/sv_shadow.vs");
    	shadow_prog.compileShader("data/shaders/sv_shadow.fs");
    	shadow_prog.link();
    	shadow_prog.validate();
    }
    catch(GLSLProgramException & e)
    {
 		cerr<<e.what()<<endl;
 		exit(EXIT_FAILURE);
    }
}

int Clear()
{
    delete vase;
    delete torus;
    delete plane;
    delete cube;
    delete sphere;

    return 0;
}

int SetupFBO()
{
    // depth buffer
    GLuint depth_buffer;
    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // ambient light buffer
    GLuint ambient_buffer;
    glGenRenderbuffers(1, &ambient_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, ambient_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

    // diffuse+specular light
    glActiveTexture(GL_TEXTURE0);
    //GLuint diffuse_specular_texture;
    glGenTextures(1, &diffuse_specular_texture);
    glBindTexture(GL_TEXTURE_2D, diffuse_specular_texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // create and setup the FBO
    // depth to depth buffer
    // ambient light to color 0
    // diffuse+specular light to color 1
    glGenFramebuffers(1, &color_depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, color_depth_fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ambient_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffuse_specular_texture, 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( result == GL_FRAMEBUFFER_COMPLETE) {
        cout<<"Framebuffer is complete.\n";
    } else {
        cout<<"Framebuffer is not complete.\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    return 0;
}

int InitScene()
{
    CompileAndLinkShader();

    vase = new Model("data/models/vase.dae");
    torus = new Model("data/models/torus.dae");
    plane = new Model("data/models/plane.dae");
    cube = new Model("data/models/cube.dae");
    sphere = new Model("data/models/sphere.dae");

    eye = vec3(2.0f, 8.0f, 12.0f);
    look = vec3(0.0f, 0.0f, 0.0f);
    up = vec3(0.0f, 1.0f, 0.0f);

    light_eye = vec3(-6.0f, 30.0f, 10.0f);
    light_look = vec3(0.0f,0.0f,0.0f);
    light_up = vec3(0.0f,1.0f,0.0f);

    SetupFBO();

    // setup a dummy vao
    GLfloat verts[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f};
    GLuint dummy_buffer;
    glGenBuffers(1, &dummy_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, dummy_buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    glGenVertexArrays(1, &dummy_vao);
    glBindVertexArray(dummy_vao);

    glBindBuffer(GL_ARRAY_BUFFER, dummy_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindVertexArray(0);

    return 0;
}

void SetMatrices(GLSLProgram &prog)
{
    mat4 mv = view * model;
    prog.setUniform("mv", mv);
    prog.setUniform("normal_transform", mat3(glm::transpose(glm::inverse(mv))));
    prog.setUniform("mvp", projection * mv);
}

int DrawScene(GLSLProgram &prog, bool isVolumePass)
{
#if 1
    //cout<<"DrawScene 1"<<endl;
    model = glm::rotate(glm::radians(-angle), glm::vec3(0.f, 1.f, 0.f));
    SetMatrices(prog);
    //cout<<"DrawScene 1.1"<<endl;

    if(!isVolumePass)
    {
        prog.setUniform("material.kd", 0.4f, 0.2f, 0.7f);
        prog.setUniform("material.ka", 0.4f, 0.3f, 0.6f);
        prog.setUniform("material.ks", 0.9f, 0.9f, 0.9f);
        prog.setUniform("material.shininess", 10.0f);
        vase->Render();
    }
    else
        vase->RenderAsTrianglesAdj();
#endif

#if 1
    //cout<<"DrawScene 2"<<endl;
    model = glm::rotate(glm::radians(-angle), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(vec3(6.f, 6.f, 0.f));
    SetMatrices(prog);
    if(!isVolumePass)
    {
        prog.setUniform("material.kd", 0.9f, 0.2f, 0.2f);
        prog.setUniform("material.ka", 0.7f, 0.2f, 0.2f);
        prog.setUniform("material.ks", 0.8f, 0.8f, 0.8f);
        prog.setUniform("material.shininess", 5.0f);
        torus->Render();
    }
    else
        torus->RenderAsTrianglesAdj();
#endif

#if 1
    //cout<<"DrawScene 3"<<endl;
    model = glm::rotate(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(vec3(5.f, 3.5f, 0.f));
    SetMatrices(prog);
    if(!isVolumePass)
    {
        prog.setUniform("material.kd", 0.2f, 0.2f, 0.9f);
        prog.setUniform("material.ka", 0.1f, 0.2f, 0.6f);
        prog.setUniform("material.ks", 0.8f, 0.8f, 0.8f);
        prog.setUniform("material.shininess", 10.0f);
        cube->Render();
    }
    else
        cube->RenderAsTrianglesAdj();
#endif

#if 1
    model = glm::rotate(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)) * glm::translate(vec3(3.f, 10.f, 0.f));
    SetMatrices(prog);
    if(!isVolumePass)
    {
        prog.setUniform("material.kd", 0.5f, 0.9f, 0.1f);
        prog.setUniform("material.ka", 0.8f, 0.6f, 0.2f);
        prog.setUniform("material.ks", 0.8f, 0.8f, 0.8f);
        prog.setUniform("material.shininess", 10.0f);
        sphere->Render();
    }
    else
        sphere->RenderAsTrianglesAdj();
#endif

#if 1
    if(!isVolumePass)
    {
        //cout<<"DrawScene 4"<<endl;
        model = glm::translate(vec3(0.f, -10.f, 0.f));
        SetMatrices(prog);
        prog.setUniform("material.kd", 0.3f, 0.6f, 0.3f);
        prog.setUniform("material.ka", 0.3f, 0.6f, 0.2f);
        prog.setUniform("material.ks", 0.2f, 0.2f, 0.2f);
        prog.setUniform("material.shininess", 55.0f);
        plane->Render();
    }
#endif

    return 0;
}

int Pass1()
{
    // render normally.
    glBindFramebuffer(GL_FRAMEBUFFER, color_depth_fbo);
    glBindTexture(GL_TEXTURE_2D, diffuse_specular_texture);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    view = glm::lookAt(eye, look, up);

    scene_prog.use();
    scene_prog.setUniform("light.intensity", vec3(0.85f));
    scene_prog.setUniform("light.position", view * vec4(light_eye, 1.0f));
    DrawScene(scene_prog, false);

    return 0;
}

int Pass2()
{
    // copy color_depth_fbo to default frame buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, color_depth_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width - 1, height - 1, 0, 0, width - 1, height - 1, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(!show_volume)
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
    }
    else
    {
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
    }

    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    // make stencil test always pass
    glStencilFunc(GL_ALWAYS, 0x0, 0xFFFF);
    // We draw shadow volume in this pass.
    // Note that we already got the depth buffer of the whole scene in pass 1.
    // Now for a front face that passes the depth test(between eye and a scene object if any), do a GL_INCR_WRAP(+1).
    // For a back face that passes the depth test, do a GL_DECR_WRAP(-1).
    // So for a shadow volume mesh, if we finally get a 0 in stencil buffer,
    // the scene object is either nearer than the front face of the shadow volume(all fail),
    // or further than the back face of the shadow volume(all pass).
    // That is, the object is not in the shadow volume.
    // If the object is in the shadow volume, we will get an 1(front pass, back fail).
    // This principle also holds for multiple shadow volume.
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK,  GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    volume_prog.use();

    volume_prog.setUniform("projection_matrix", projection);
    volume_prog.setUniform("light_position", view * vec4(light_eye, 1.0f));

    DrawScene(volume_prog, true);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    return 0;
}

int Pass3()
{
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);


    glStencilFunc(GL_EQUAL, 0x0, 0xFFFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    shadow_prog.use();

    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);

    //projection = glm::infinitePerspective(glm::radians(90.0f), (float)width / height, 0.5f);
    projection = glm::perspective(glm::radians(90.0f), (float)width / height, 1.f, 400.0f);
    glDisable(GL_BLEND);

    return 0;
}

int Render()
{
    Pass1();
    Pass2();
    Pass3();

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
    //projection = glm::infinitePerspective(glm::radians(90.0f), (float)width / height, 0.5f);

    return 0;
}

int DoImgui()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    if(!ImGui::Begin("shadowmap", NULL, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::End();
    }
    ImGui::Text("space : toggle rotation\nesc : exit\np : toggle eye position\nv : toggle volume rendering");
    ImGui::End();

    ImGui::Render();

    if(ImGui::IsKeyPressed(GLFW_KEY_SPACE))
        rotating = 1 - rotating;
    if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GL_TRUE);
    if(ImGui::IsKeyPressed(GLFW_KEY_P))
    {
        if(eye_position)
            eye = vec3(5.0f, 10.0f, 12.0f);
        else
            eye = vec3(5.0f, 4.0f, 5.0f);
        eye_position = 1 - eye_position;
    }
    if(ImGui::IsKeyPressed(GLFW_KEY_V))
        show_volume = 1 - show_volume;

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
	window = glfwCreateWindow(width, height, "shadow volume (z-pass)", NULL, NULL);
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

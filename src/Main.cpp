#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>

sf::RenderWindow window;
const int FPS = 60;

GLuint vertShader, fragShader;
GLuint cubeVertShader, cubeFragShader;
GLuint planeVertShader, planeFragShader;
GLuint playerVertShader, playerFragShader;
GLuint sphereProgram;
GLuint cubeProgram;
GLuint planeProgram;

//obj files blender export set to *Triangulate*
std::string objFile = "untitled1.ply";
int objPointCount = 0; //number of points in obj
int playerPointCount = 0;

GLuint vao;
GLuint cubeVao;
GLuint planeVao;

//texture
GLuint textureID;
sf::Image upTex;
sf::Image downTex;
sf::Image leftTex;
sf::Image rightTex;
sf::Image frontTex;
sf::Image backTex;

sf::Texture grassTex;

//heightmap
sf::Image heightmap;

//camera
glm::mat4 viewMat;
glm::mat4 projMat;
glm::vec3 camPos(1000.0f, 50.0f, 1000.0f);
glm::vec3 playerCamPos(1000.0f, 50.0f, 1000.0f);
float nearPlane = 0.1f; //clipping plane
float farPlane = 1800.f; //clipping plane
float fovy = 67.f; //67 degrees
float aspect = 4/3;
float camSpeed = 10.5f; //movement speed
float camHeadingSpeed = 4.f; //turning speed
float camHeading = 0.f; //y rotation in degrees
float pitch = 0;

glm::vec3 playerPos;
glm::mat4 posMat;

glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-camPos[0], -camPos[1], -camPos[2]));
glm::mat4 rotate = glm::rotate(glm::mat4(1.0), -camHeading, glm::vec3(0.0, 1.0, 0.0));
glm::quat q = glm::angleAxis(-camHeading, glm::vec3(0.f, 1.f, 0.f));

GLuint makeBigCube() {
	float points[] = {
		-1000.f, 1000.f, -1000.f,
		-1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,
		1000.f, 1000.f, -1000.f,
		-1000.f, 1000.f, -1000.f,

		-1000.f, -1000.f, 1000.f,
		-1000.f, -1000.f, -1000.f,
		-1000.f, 1000.f, -1000.f,
		-1000.f, 1000.f, -1000.f,
		-1000.f, 1000.f, 1000.f,
		-1000.f, -1000.f, 1000.f,

		1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, 1000.f,
		1000.f, 1000.f, 1000.f,
		1000.f, 1000.f, 1000.f,
		1000.f, 1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,

		-1000.f, -1000.f, 1000.f,
		-1000.f, 1000.f, 1000.f,
		1000.f, 1000.f, 1000.f,
		1000.f, 1000.f, 1000.f,
		1000.f, -1000.f, 1000.f,
		-1000.f, -1000.f, 1000.f,

		-1000.f, 1000.f, -1000.f,
		1000.f, 1000.f, -1000.f,
		1000.f, 1000.f, 1000.f,
		1000.f, 1000.f, 1000.f,
		-1000.f, 1000.f, 1000.f,
		-1000.f, 1000.f, -1000.f,

		-1000.f, -1000.f, -1000.f,
		-1000.f, -1000.f, 1000.f,
		1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,
		-1000.f, -1000.f, 1000.f,
		1000.f, -1000.f, 1000.f
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof (GLfloat), &points, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	return vao;
}

struct VertexPos
{
	glm::vec3 pos;
};

struct TexPos
{
	GLfloat x;
	GLfloat y;
};

struct Heights
{
	float height;
};

std::vector<VertexPos> verts;
std::vector<TexPos> texCoords;
std::vector<Heights> heights;
GLuint makePlane()
{
	float size = 15;
	int quads = heightmap.getSize().x;
	for (int i = 0; i < quads; ++i)
	{
		for (int j = 0; j < quads; ++j)
		{
			//------------------Plane and heightmap------------------------------
			sf::Color color = heightmap.getPixel(i, j);
			float height = ((float)color.r + (float)color.g + (float)color.b) / 10;
			verts.push_back({ glm::vec3((i * size), height, (j * size)) });
			heights.push_back({ height });

			color = heightmap.getPixel(i, j + 1);
			height = ((float)color.r + (float)color.g + (float)color.b) / 10;
			verts.push_back({ glm::vec3((i * size), height, (j * size) + size) });
			heights.push_back({ height });

			color = heightmap.getPixel(i + 1, j);
			height = ((float)color.r + (float)color.g + (float)color.b) / 10;
			verts.push_back({ glm::vec3((i * size) + size, height, (j * size)) });
			heights.push_back({ height });

			color = heightmap.getPixel(i + 1, j);
			height = ((float)color.r + (float)color.g + (float)color.b) / 10;
			verts.push_back({ glm::vec3((i * size) + size, height, (j * size)) });
			heights.push_back({ height });

			color = heightmap.getPixel(i, j + 1);
			height = ((float)color.r + (float)color.g + (float)color.b) / 10;
			verts.push_back({ glm::vec3((i * size), height, (j * size) + size) });
			heights.push_back({ height });

			color = heightmap.getPixel(i + 1, j + 1);
			height = ((float)color.r + (float)color.g + (float)color.b) / 10;
			verts.push_back({ glm::vec3((i * size) + size, height, (j * size) + size) });
			heights.push_back({ height });

			//----------------------------Textures--------------------------------------
			texCoords.push_back({ 0, 0 });
			texCoords.push_back({ 0, 1 });
			texCoords.push_back({ 1, 0 });
			texCoords.push_back({ 1, 0 });
			texCoords.push_back({ 0, 1 });
			texCoords.push_back({ 1, 1 });
		}
	}

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(VertexPos), verts.data(), GL_STATIC_DRAW);

	GLuint vboTex;
	glGenBuffers(1, &vboTex);
	glBindBuffer(GL_ARRAY_BUFFER, vboTex);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(TexPos), texCoords.data(), GL_STATIC_DRAW);

	GLuint vboHeight;
	glGenBuffers(1, &vboHeight);
	glBindBuffer(GL_ARRAY_BUFFER, vboHeight);
	glBufferData(GL_ARRAY_BUFFER, heights.size() * sizeof(Heights), heights.data(), GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vboTex);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vboHeight);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);

	return vao;
}

void loadSkyBoxSide(int target)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	switch (target)
	{
	case 0:
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA,
			frontTex.getSize().x, frontTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, frontTex.getPixelsPtr());
		break;
	case 1:
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA,
			backTex.getSize().x, backTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, backTex.getPixelsPtr());
		break;
	case 2:
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA,
			upTex.getSize().x, upTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, upTex.getPixelsPtr());
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA,
			downTex.getSize().x, downTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, downTex.getPixelsPtr());
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA,
			leftTex.getSize().x, leftTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, leftTex.getPixelsPtr());
		break;
	case 5:
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA,
			rightTex.getSize().x, rightTex.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rightTex.getPixelsPtr());
		break;
	}
}

void init()
{
	//load texture
	upTex.loadFromFile("skybox_up.png");
	downTex.loadFromFile("skybox_down.png");
	frontTex.loadFromFile("skybox_front.png");
	backTex.loadFromFile("skybox_back.png");
	leftTex.loadFromFile("skybox_left.png");
	rightTex.loadFromFile("skybox_right.png");

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);

	for (int i = 0; i < 6; ++i)
		loadSkyBoxSide(i);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	grassTex.loadFromFile("grass.png");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	heightmap.loadFromFile("heightmap.png");
}

bool load_mesh(const char* file_name, GLuint* vao, int* point_count) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate);
	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	printf("  %i animations\n", scene->mNumAnimations);
	printf("  %i cameras\n", scene->mNumCameras);
	printf("  %i lights\n", scene->mNumLights);
	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	/* get first mesh in file only */
	const aiMesh* mesh = scene->mMeshes[0];
	printf("    %i vertices in mesh[0]\n", mesh->mNumVertices);

	/* pass back number of vertex points in mesh */
	*point_count = mesh->mNumVertices;

	/* generate a VAO, using the pass-by-reference parameter that we give to the
	function */
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	GLfloat* points = NULL; // array of vertex points
	GLfloat* normals = NULL; // array of vertex normals
	GLfloat* texcoords = NULL; // array of texture coordinates
	if (mesh->HasPositions()) {
		points = (GLfloat*)malloc(*point_count * 3 * sizeof(GLfloat));
		for (int i = 0; i < *point_count; i++) {
			const aiVector3D* vp = &(mesh->mVertices[i]);
			points[i * 3] = (GLfloat)vp->x;
			points[i * 3 + 1] = (GLfloat)vp->y;
			points[i * 3 + 2] = (GLfloat)vp->z;
		}
	}
	if (mesh->HasNormals()) {
		normals = (GLfloat*)malloc(*point_count * 3 * sizeof(GLfloat));
		for (int i = 0; i < *point_count; i++) {
			const aiVector3D* vn = &(mesh->mNormals[i]);
			normals[i * 3] = (GLfloat)vn->x;
			normals[i * 3 + 1] = (GLfloat)vn->y;
			normals[i * 3 + 2] = (GLfloat)vn->z;
		}
	}
	if (mesh->HasTextureCoords(0)) {
		texcoords = (GLfloat*)malloc(*point_count * 2 * sizeof(GLfloat));
		for (int i = 0; i < *point_count; i++) {
			const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
			texcoords[i * 2] = (GLfloat)vt->x;
			texcoords[i * 2 + 1] = (GLfloat)vt->y;
		}
	}

	/* copy mesh data into VBOs */
	if (mesh->HasPositions()) {
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			3 * *point_count * sizeof(GLfloat),
			points,
			GL_STATIC_DRAW
			);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
		free(points);
	}
	if (mesh->HasNormals()) {
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			3 * *point_count * sizeof(GLfloat),
			normals,
			GL_STATIC_DRAW
			);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);
		free(normals);
	}
	if (mesh->HasTextureCoords(0)) {
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			2 * *point_count * sizeof(GLfloat),
			texcoords,
			GL_STATIC_DRAW
			);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(2);
		free(texcoords);
	}
	if (mesh->HasTangentsAndBitangents()) {
		// NB: could store/print tangents here
	}

	aiReleaseImport(scene);
	printf("mesh loaded\n");

	return true;
}

bool compileS(GLuint obj)
{
	int status;
	glGetShaderiv(obj, GL_COMPILE_STATUS, &status);

	if (status == GL_TRUE)
		return true;

	return false;
}

bool linkS(GLuint obj)
{
	int status;
	glGetShaderiv(obj, GL_LINK_STATUS, &status);

	if (status == GL_TRUE)
		return true;

	return false;
}

void loadShader(const std::string fileName, std::string& shaderStr)
{
	std::ifstream load;
	load.open(fileName);

	if (!load.is_open())
		exit(1);

	char tmp[300];
	while (!load.eof())
	{
		load.getline(tmp, 300);
		shaderStr += tmp;
		shaderStr += '\n';
	}
	std::cout << shaderStr << std::endl;
}

void compileShader(GLuint& vertShader, GLuint& fragShader, GLuint& program)
{
	//compile and link shader program
	glCompileShader(vertShader);
	glCompileShader(fragShader);

	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
}

void makeShaders()
{
	std::string vertStr;
	std::string fragStr;
	loadShader("shader.vert", vertStr);
	loadShader("shader.frag", fragStr);

	const GLchar* vertexSrc = vertStr.c_str();
	const GLchar* fragmentSrc = fragStr.c_str();

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertShader, 1, &vertexSrc, NULL);
	glShaderSource(fragShader, 1, &fragmentSrc, NULL);

	//compile and link shader program
	glCompileShader(vertShader);
	std::cout << "vert shader " << compileS(vertShader) << std::endl;
	glCompileShader(fragShader);
	std::cout << "frag shader " << compileS(fragShader) << std::endl;

	sphereProgram = glCreateProgram();
	glAttachShader(sphereProgram, vertShader);
	glAttachShader(sphereProgram, fragShader);
	glLinkProgram(sphereProgram);
	std::cout << "program link " << linkS(sphereProgram) << std::endl;

	//clean up
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

void makeShaders1()
{
	std::string vertStr;
	std::string fragStr;
	loadShader("skybox.vert", vertStr);
	loadShader("skybox.frag", fragStr);

	const GLchar* vertexSrc = vertStr.c_str();
	const GLchar* fragmentSrc = fragStr.c_str();

	cubeVertShader = glCreateShader(GL_VERTEX_SHADER);
	cubeFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(cubeVertShader, 1, &vertexSrc, NULL);
	glShaderSource(cubeFragShader, 1, &fragmentSrc, NULL);

	//compile and link shader program
	glCompileShader(cubeVertShader);
	std::cout << "vert shader " << compileS(cubeVertShader) << std::endl;
	glCompileShader(cubeFragShader);
	std::cout << "frag shader " << compileS(cubeFragShader) << std::endl;

	cubeProgram = glCreateProgram();
	glAttachShader(cubeProgram, cubeVertShader);
	glAttachShader(cubeProgram, cubeFragShader);
	glLinkProgram(cubeProgram);
	std::cout << "program link " << linkS(cubeProgram) << std::endl;

	//clean up
	glDeleteShader(cubeVertShader);
	glDeleteShader(cubeFragShader);
}

void makeShaders2()
{
	std::string vertStr;
	std::string fragStr;
	loadShader("plane.vert", vertStr);
	loadShader("plane.frag", fragStr);

	const GLchar* vertexSrc = vertStr.c_str();
	const GLchar* fragmentSrc = fragStr.c_str();

	planeVertShader = glCreateShader(GL_VERTEX_SHADER);
	planeFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(planeVertShader, 1, &vertexSrc, NULL);
	glShaderSource(planeFragShader, 1, &fragmentSrc, NULL);

	//compile and link shader program
	glCompileShader(planeVertShader);
	std::cout << "vert shader " << compileS(planeVertShader) << std::endl;
	glCompileShader(planeFragShader);
	std::cout << "frag shader " << compileS(planeFragShader) << std::endl;

	planeProgram = glCreateProgram();
	glAttachShader(planeProgram, planeVertShader);
	glAttachShader(planeProgram, planeFragShader);
	glLinkProgram(planeProgram);
	std::cout << "program link " << linkS(planeProgram) << std::endl;

	//clean up
	glDeleteShader(planeVertShader);
	glDeleteShader(planeFragShader);
}

void makeShaders3()
{
	std::string vertStr;
	std::string fragStr;
	loadShader("player.vert", vertStr);
	loadShader("player.frag", fragStr);

	const GLchar* vertexSrc = vertStr.c_str();
	const GLchar* fragmentSrc = fragStr.c_str();

	playerVertShader = glCreateShader(GL_VERTEX_SHADER);
	playerFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(playerVertShader, 1, &vertexSrc, NULL);
	glShaderSource(playerFragShader, 1, &fragmentSrc, NULL);

	//compile and link shader program
	glCompileShader(playerVertShader);
	std::cout << "vert shader " << compileS(playerVertShader) << std::endl;
	glCompileShader(playerFragShader);
	std::cout << "frag shader " << compileS(playerFragShader) << std::endl;

	//clean up
	glDeleteShader(playerVertShader);
	glDeleteShader(playerFragShader);
}

void render()
{
	glViewport(0, 0, window.getSize().x, window.getSize().y);
	glClearColor(.4f, .4f, .4f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	//1st program
	glUseProgram(sphereProgram);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, objPointCount);

	//2nd program
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthMask(GL_FALSE);
	glUseProgram(cubeProgram);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glBindVertexArray(cubeVao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

	//3rd program
	glFrontFace(GL_CCW);
	glEnable(GL_TEXTURE_2D);
	sf::Texture::bind(&grassTex);
	glUseProgram(planeProgram);
	glBindVertexArray(planeVao);
	glDrawArrays(GL_TRIANGLES, 0, verts.size());
	//-----------------------

	window.display();
	//window.setFramerateLimit(FPS);
	window.setVerticalSyncEnabled(true);
}

void displayFps(sf::Clock& clock, sf::Time& time, int& fps)
{
	++fps;
	if (time.asSeconds() >= 1)
	{
		COORD pos = { 0, 35 };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
		std::cout << "Frames per second = " << fps;
		fps = 0;
		clock.restart();
	}
}

void trapMouse(sf::RenderWindow& window, sf::Vector2i& lastMouse)
{
	sf::Mouse::setPosition(sf::Vector2i((window.getPosition().x + window.getSize().x) / 2, 
		(window.getPosition().y + window.getSize().y) / 2));

	lastMouse = sf::Mouse::getPosition();
}

int main()
{
	sf::Event event;
	sf::Clock clock;
	sf::Time time;
	sf::Vector2i mouse(0, 0), lastMouse(0, 0);
	int mouseScrollLast = 0;
	float zoom = 10;
	glewInit();

	int fps = 0;
	window.create(sf::VideoMode(800, 600), "OpenGL");
	window.setMouseCursorVisible(false);
	
	init();

	load_mesh(objFile.c_str(), &vao, &objPointCount);
	load_mesh(playerObjFile.c_str(), &playerVao, &playerPointCount);
	cubeVao = makeBigCube();
	planeVao = makePlane();

	makeShaders();
	makeShaders1();
	makeShaders2();

	//camera
	aspect = (float)window.getSize().x / (float)window.getSize().y;
	projMat = glm::perspective(fovy, aspect, nearPlane, farPlane);
	viewMat = rotate * translate;

	//vectors for keyboard movement
	glm::vec4 fwd(0.0f, 0.0f, -1.0f, 0.0f); 
	glm::vec4 rgt(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 up(0.0f, 1.0f, 0.0f, 0.0f);

	//needed to keep camera from tilting when changing yaw
	const glm::vec4 UP(0.0f, 1.0f, 0.0f, 0.0f);

	//uniforms
	GLuint projID = glGetUniformLocation(sphereProgram, "proj");
	GLuint viewID = glGetUniformLocation(sphereProgram, "view");

	GLuint cubeProjID = glGetUniformLocation(cubeProgram, "proj");
	GLuint cubeViewID = glGetUniformLocation(cubeProgram, "view");

	GLuint planeProjID = glGetUniformLocation(planeProgram, "proj");
	GLuint planeViewID = glGetUniformLocation(planeProgram, "view");

	GLuint playerProjID = glGetUniformLocation(playerProgram, "proj");
	GLuint playerViewID = glGetUniformLocation(playerProgram, "view");
	GLuint posID = glGetUniformLocation(playerProgram, "pos");

	glUseProgram(sphereProgram);
	glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(projMat));
	glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewMat));

	glUseProgram(cubeProgram);
	glUniformMatrix4fv(cubeProjID, 1, GL_FALSE, glm::value_ptr(projMat));
	glUniformMatrix4fv(cubeViewID, 1, GL_FALSE, glm::value_ptr(viewMat));

	glUseProgram(planeProgram);
	glUniformMatrix4fv(planeProjID, 1, GL_FALSE, glm::value_ptr(projMat));
	glUniformMatrix4fv(planeViewID, 1, GL_FALSE, glm::value_ptr(viewMat));
	//------------------------------------
	bool paused = false;
	bool jump = false;
	sf::Time jumpTimer;
	sf::Clock jumpClock;

	while (window.isOpen())
	{
		time = clock.getElapsedTime();
		bool camMoved = false;
		glm::vec3 move(0.0, 0.0, 0.0);
		float camYaw = 0.f;
		float camPitch = 0.f;
		float camRoll = 0.f;

		jumpTimer = jumpClock.getElapsedTime();

		mouse = sf::Mouse::getPosition();

		if (paused == false)
			trapMouse(window, lastMouse);

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					window.close();

				if (event.key.code == sf::Keyboard::P)
					paused = !paused;
			}

			if (event.type == sf::Event::MouseWheelMoved)
			{
				if (event.mouseWheel.delta < 0 && zoom < 20)
				{
					zoom += 2;
				}
				else if (event.mouseWheel.delta > 0 && zoom > 4)
				{
					zoom -= 2;
				}
			}
		}

		if (paused == false)
		{
			//movement
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			{
				move[0] -= camSpeed;
				camMoved = true;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			{
				move[0] += camSpeed;
				camMoved = true;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				move[2] -= camSpeed;
				camMoved = true;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{
				move[2] += camSpeed;
				camMoved = true;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && jump == false)
			{
				jump = true;
				camMoved = true;
				jumpClock.restart();
			}

			//up down
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
			{
				move[1] += camSpeed;
				camMoved = true;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
			{
				move[1] -= camSpeed;
				camMoved = true;
			}

			//turning
			if (mouse.y < lastMouse.y && pitch <= 180)
			{
				pitch += camHeadingSpeed * 2;
				camPitch += camHeadingSpeed;
				camMoved = true;
				glm::quat qPitch = glm::angleAxis(camPitch, glm::vec3(rgt[0], rgt[1], rgt[2]));
				q = qPitch * q;
			}
			else if (mouse.y > lastMouse.y && pitch >= -180)
			{
				pitch -= camHeadingSpeed * 2;
				camPitch -= camHeadingSpeed;
				camMoved = true;
				glm::quat qPitch = glm::angleAxis(camPitch, glm::vec3(rgt[0], rgt[1], rgt[2]));
				q = qPitch * q;
			}
			if (mouse.x < lastMouse.x)
			{
				camYaw += camHeadingSpeed;
				camMoved = true;
				glm::quat qYaw = glm::angleAxis(camYaw, glm::vec3(UP[0], UP[1], UP[2]));
				q = qYaw * q;
			}
			else if (mouse.x > lastMouse.x)
			{
				camYaw -= camHeadingSpeed;
				camMoved = true;
				glm::quat qYaw = glm::angleAxis(camYaw, glm::vec3(UP[0], UP[1], UP[2]));
				q = qYaw * q;
			}
		}

		playerCamPos = playerCamPos + glm::vec3(fwd) * -move[2];
		playerCamPos = playerCamPos + glm::vec3(up) * -move[1];
		playerCamPos = playerCamPos + glm::vec3(rgt) * move[0];

		camPos = playerCamPos;
		camPos.z = camPos.z + zoom * 1.5;
		camPos.y = camPos.y + zoom / 2;

		if (jump)
		{
			camPos[1] += .9 - 2.5 * (jumpTimer.asMilliseconds() / 900);

			if (jumpTimer.asMilliseconds() >= 300)
			{
				jump = false;
				jumpClock.restart();
			}
		}

		//camera update
		{		
			rotate = glm::toMat4(q); //quat to mat4
			fwd = rotate * glm::vec4(0.0, 0.0, -1.0, 0.0);
			rgt = rotate * glm::vec4(1.0, 0.0, 0.0, 0.0);
			up = rotate * glm::vec4(0.0, 1.0, 0.0, 0.0);

			translate = glm::translate(glm::mat4(1.0), glm::vec3(camPos));
			viewMat = glm::inverse(rotate) * glm::inverse(translate);

			glUseProgram(sphereProgram);
			glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewMat));

			glUseProgram(planeProgram);
			glUniformMatrix4fv(planeViewID, 1, GL_FALSE, glm::value_ptr(viewMat));

			glUseProgram(cubeProgram);
			glUniformMatrix4fv(cubeViewID, 1, GL_FALSE, glm::value_ptr(glm::inverse(rotate)));
		}
		//sphere color change
		glUseProgram(sphereProgram);
		GLuint colorID = glGetUniformLocation(sphereProgram, "color");
		glUniform1f(colorID, time.asMilliseconds() * .01);

		render();
		displayFps(clock, time, fps);
	}
	return 0;                                                                                                                     
}
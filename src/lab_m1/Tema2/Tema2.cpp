#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

#define RADIANS(angle)      ((angle) * 0.0174532925194444f)

Tema2::Tema2()
{
    distanceToTarget = 2.5;
    isThirdPerson = 1;
    angleCharacter = 0;
    heightCharacter = 0;
    fov = 60;
    zNear = 0.01f;
    zFar = 200.0f;
    attackMode = false;
    life = 2;
}


Tema2::~Tema2()
{
}

void Tema2::Init()
{
    float posX = -2, posY = 1.5, posZ = 0;
    cameraThirdPerson = new implemented::Camera();
    cameraThirdPerson->Set(glm::vec3(posX, posY, posZ), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), distanceToTarget);

    cameraFirstPerson = new implemented::Camera();
    cameraFirstPerson->Set(glm::vec3(posX, posY, posZ), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), 0);
    cameraFirstPerson->MoveForward(distanceToTarget * 0.8);

    camera = cameraThirdPerson;

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("bunny");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "animals"), "bunny.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, zNear, zFar);
    CreateCube("head", glm::vec3(0.85, 0.79, 0.72));
    CreateCube("torso", glm::vec3(0.5, 0.7, 0.7));
    CreateCube("legs", glm::vec3(0.3, 0.3, 0.6));
    CreateCube("floor", glm::vec3(0.5, 0.5, 0.5));
    CreateCube("wall", glm::vec3(0.8, 0.5, 0.25));

    // Square
    {
        std::vector<VertexFormat> vertices
        {
            VertexFormat(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)),
            VertexFormat(glm::vec3(1, 0, 0), glm::vec3(1, 1, 1)),
            VertexFormat(glm::vec3(1, 1, 0), glm::vec3(1, 1, 1)),
            VertexFormat(glm::vec3(0, 1, 0), glm::vec3(1, 1, 1)),
        };

        std::vector<unsigned int> indices =
        {
            0, 1, 2,
            0, 2, 3
        };

        CreateMesh("bar", vertices, indices);
    }

    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // MAZE
    for (int i = 0; i < mazeDim; i++) {
        for (int j = 0; j < mazeDim; j++) {
            if (maze[i][j] == 1) {
                // WALLS
                wall* newWall = new wall(j * 4 - 4, i * 4 - 4, 2);
                wallsVector.push_back(*newWall);
            } else if (maze[i][j] == 2) {
                // INAMICI
                enemy* newEnemy = new enemy(j * 4 - 5.5, 1, i * 4 - 5.5, 0.1, 3, 0);
                enemiesVector.push_back(*newEnemy);
            }
        }
    }
   
}

// creeaza un mesh in forma unui cub cu nume si culoare date
void Tema2::CreateCube(char* name, glm::vec3 color)
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-1, -1,  1), glm::vec3(0, 1, 1), color),
        VertexFormat(glm::vec3(1, -1,  1), glm::vec3(1, 0, 1), color),
        VertexFormat(glm::vec3(-1,  1,  1), glm::vec3(1, 0, 0), color),
        VertexFormat(glm::vec3(1,  1,  1), glm::vec3(0, 1, 0), color),
        VertexFormat(glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1), color),
        VertexFormat(glm::vec3(1, -1, -1), glm::vec3(0, 1, 1), color),
        VertexFormat(glm::vec3(-1,  1, -1), glm::vec3(1, 1, 0), color),
        VertexFormat(glm::vec3(1,  1, -1), glm::vec3(0, 0, 1), color),
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,
        1, 3, 2,
        2, 3, 7,
        2, 7, 6,
        1, 7, 3,
        1, 5, 7,
        6, 7, 4,
        7, 5, 4,
        0, 4, 1,
        1, 4, 5,
        2, 6, 4,
        0, 2, 4,
    };

    CreateMesh(name, vertices, indices);
}

bool Tema2::CheckWallCollision(float newX, float newZ, float length)
{
    for (int i = 0; i < wallsVector.size(); i++) {
        // verific ca niciunul din colturile figurii sa se afle in interiorul obstacolului
        if (PointInArea(newX - length, newZ - length, &wallsVector[i]) ||
            PointInArea(newX + length, newZ - length, &wallsVector[i]) ||
            PointInArea(newX - length, newZ + length, &wallsVector[i]) ||
            PointInArea(newX + length, newZ + length, &wallsVector[i])) {

            return true;
        }
    }
    // nu exista coliziuni intre obstacole si obiectul verificat
    return false;
}

bool Tema2::CheckEnemyCollision(float newX, float newZ, float length)
{
    float first, second, collisionDistance;
    for (int i = 0; i < enemiesVector.size(); i++) {
        // distanta euclidiana dintre cele 2 centre
        first = pow(enemiesVector[i].x - newX, 2);
        second = pow(enemiesVector[i].z - newZ, 2);
        collisionDistance = sqrt(first + second);
        if (collisionDistance <= length) {
            enemiesVector.erase(enemiesVector.begin() + i);
            return true;
        }
    }
    // nu exista coliziuni intre inamici si obiectul verificat
    return false;
}

void Tema2::CheckProjectileCollision()
{
    for (int i = 0; i < projectilesVector.size(); i++) {
        // coliziune proiectil-zid / proiectil-inamic
        if (CheckWallCollision(projectilesVector[i].x, projectilesVector[i].z, 0.1) ||
            CheckEnemyCollision(projectilesVector[i].x, projectilesVector[i].z, 1.2)) {

            projectilesVector.erase(projectilesVector.begin() + i);
            i--;
        }
    }
}

// verifica daca punctul determinat de cele 2 coordonate se afla in interiorul obstacolului dat ca parametru
bool Tema2::PointInArea(float posX, float posZ, wall* wall)
{
    float obsX = wall->centerX;
    float obsZ = wall->centerZ;
    if ((posZ <= obsZ + 2) && (posZ >= obsZ - 2) &&
        (posX <= obsX + 2) && (posX >= obsX - 2)) {
        return true;
    }
    return false;
}

Mesh* Tema2::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    glBindVertexArray(0);

    CheckOpenGLError();

    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    meshes[name]->vertices = vertices;
    meshes[name]->indices = indices;
    return meshes[name];
}


void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
    window->DisablePointer();
    // MAZE, INAMICI, PROIECTILE
    DrawMaze();
    DrawEnemies();
    DrawProjectiles();

    glm::vec3 pos = camera->GetTargetPosition();
    // PLAYER
    if (isThirdPerson == 1) {
        // desenare player
        DrawPlayer(pos);
        // healthbar TPS
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
            modelMatrix *= transform3D::RotateOY(RADIANS(90));
            modelMatrix *= transform3D::RotateOY(-angleCharacter);
            modelMatrix *= transform3D::Translate(0.8, 0.8, 0);
            modelMatrix *= transform3D::Scale(0.5 * life, 0.2, 0);
            RenderMesh(meshes["bar"], shaders["LabShader"], modelMatrix);
        }
    } else {
        // healthbar FPS
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
            modelMatrix *= transform3D::RotateOY(RADIANS(90));
            modelMatrix *= transform3D::RotateOY(-angleCharacter);
            modelMatrix *= transform3D::Translate(0.5, 0.3, 1.5);
            modelMatrix *= transform3D::Scale(0.45 * life, 0.2, 0);
            RenderMesh(meshes["bar"], shaders["LabShader"], modelMatrix);
        }
    }
    // coliziunile posibile pentru proiectile (proiectil-perete, proiectil-inamic)
    CheckProjectileCollision();
    // coliziunea inamic-player
    if (CheckEnemyCollision(pos.x, pos.z, 1)) {
        life--;
        if (life <= 0) {
            printf("------------------------------\n");
            printf("GAME OVER! BETTER LUCK NEXT TIME!\n");
            printf("------------------------------\n");
            World::Exit();
        }
    }
    // verific daca am gasit iesirea
    if (pos.x > (mazeDim - 2) * 4 + 2 || pos.z > (mazeDim - 2) * 4 + 2 ||
        pos.x < -4 || pos.z < -4) {
        printf("------------------------------\n");
        printf("CONGRATULATIONS, YOU FOUND THE EXIT!\n");
        printf("------------------------------\n");
        World::Exit();
    }
}

void Tema2::DrawEnemies()
{
    for (int i = 0; i < enemiesVector.size(); i++) {
        if (enemiesVector[i].distance <= 0) {
            enemiesVector[i].distance = 3;
            // urmatoarea latura, fiecare fiind codificata cu cifre 0-3
            enemiesVector[i].direction = enemiesVector[i].direction == 3 ? 0 : enemiesVector[i].direction + 1;
        }
        // verific latura pe care se afla pentru a gasi directia
        if (enemiesVector[i].direction == 0) {
            enemiesVector[i].distance -= enemiesVector[i].speed;
            enemiesVector[i].x += enemiesVector[i].speed;
        }
        else if (enemiesVector[i].direction == 1) {
            enemiesVector[i].distance -= enemiesVector[i].speed;
            enemiesVector[i].z += enemiesVector[i].speed;
        }
        else if (enemiesVector[i].direction == 2) {
            enemiesVector[i].distance -= enemiesVector[i].speed;
            enemiesVector[i].x -= enemiesVector[i].speed;
        } 
        else if (enemiesVector[i].direction == 3) {
            enemiesVector[i].distance -= enemiesVector[i].speed;
            enemiesVector[i].z -= enemiesVector[i].speed;
        }
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(enemiesVector[i].x, 0.7, enemiesVector[i].z);
        modelMatrix *= transform3D::Scale(0.05, 0.05, 0.05);
        RenderMesh(meshes["bunny"], shaders["Simple"], modelMatrix);
    }
}

void Tema2::DrawMaze()
{
    for (int i = 0; i < mazeDim; i++) {
        for (int j = 0; j < mazeDim; j++) {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= transform3D::Translate(j * 4 - 4, 0, i * 4 - 4);
            if (maze[i][j] == 1) {
                // wall
                modelMatrix *= transform3D::Scale(2, 4, 2);
                RenderMesh(meshes["wall"], shaders["VertexNormal"], modelMatrix);
            }
            else {
                // floor
                modelMatrix *= transform3D::Scale(2, 0.001, 2);
                RenderMesh(meshes["floor"], shaders["VertexNormal"], modelMatrix);
            }
        }
    }
}

void Tema2::DrawProjectiles()
{
    // desenez fiecare proiectil
    for (int i = 0; i < projectilesVector.size(); i++) {
        glm::mat4 modelMatrix = glm::mat4(1);
        // noile coordonate
        projectilesVector[i].x += 0.1 * cos(projectilesVector[i].angle) * projectilesVector[i].speed;
        projectilesVector[i].y += 0.1 * projectilesVector[i].height * projectilesVector[i].speed;
        projectilesVector[i].z += 0.1 * sin(projectilesVector[i].angle) * projectilesVector[i].speed;
        projectilesVector[i].distance += projectilesVector[i].speed;

        // a depasit distanta
        if (projectilesVector[i].distance > 40) {
            projectilesVector.erase(projectilesVector.begin() + i);
        }
        else {
            modelMatrix *= transform3D::Translate(projectilesVector[i].x, projectilesVector[i].y + 0.1, projectilesVector[i].z);
            modelMatrix *= transform3D::Scale(0.15f, 0.15f, 0.15f);
            RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
        }
    }
}

void Tema2::DrawPlayer(glm::vec3 pos)
{
    // HEAD
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, 0.17, 0);
        modelMatrix *= transform3D::Scale(0.11f, 0.11f, 0.11f);
        modelMatrix *= transform3D::RotateOY(-angleCharacter);
        RenderMesh(meshes["head"], shaders["VertexNormal"], modelMatrix);
    }

    // TORSO
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.2, 0);
        modelMatrix *= transform3D::Scale(0.1f, 0.25f, 0.2f);
        RenderMesh(meshes["torso"], shaders["VertexNormal"], modelMatrix);
    }

    // LEGS
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.73, 0.1);
        modelMatrix *= transform3D::Scale(0.1f, 0.25f, 0.085f);
        modelMatrix *= transform3D::RotateOY(-angleCharacter);
        RenderMesh(meshes["legs"], shaders["VertexNormal"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.73, -0.1);
        modelMatrix *= transform3D::Scale(0.1f, 0.25f, 0.085f);
        modelMatrix *= transform3D::RotateOY(-angleCharacter);
        RenderMesh(meshes["legs"], shaders["VertexNormal"], modelMatrix);
    }

    // SHOULDERS
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.09, -0.33);
        modelMatrix *= transform3D::Scale(0.1f, 0.14f, 0.11f);
        RenderMesh(meshes["torso"], shaders["VertexNormal"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.09, 0.33);
        modelMatrix *= transform3D::Scale(0.1f, 0.14f, 0.11f);
        RenderMesh(meshes["torso"], shaders["VertexNormal"], modelMatrix);
    }

    // ARMS
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.35, -0.33);
        modelMatrix *= transform3D::Scale(0.1f, 0.1f, 0.11f);
        RenderMesh(meshes["head"], shaders["VertexNormal"], modelMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix *= transform3D::Translate(pos.x, pos.y, pos.z);
        modelMatrix *= transform3D::Translate(0, -0.35, 0.33);
        modelMatrix *= transform3D::Scale(0.1f, 0.1f, 0.11f);
        RenderMesh(meshes["head"], shaders["VertexNormal"], modelMatrix);
    }
}


void Tema2::FrameEnd()
{
    //DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    if (window->KeyHold(GLFW_KEY_W)) {
        cameraFirstPerson->MoveForward(2 * deltaTime);
        cameraThirdPerson->MoveForward(2 * deltaTime);

        // coliziune cu perete, dau reverse la miscare
        glm::vec3 pos = cameraThirdPerson->GetTargetPosition();
        if (CheckWallCollision(pos.x, pos.z, 0.2)) {
            cameraFirstPerson->MoveForward(-2 * deltaTime);
            cameraThirdPerson->MoveForward(-2 * deltaTime);
        }
    }

    if (window->KeyHold(GLFW_KEY_A)) {
        cameraFirstPerson->TranslateRight(-2 * deltaTime);
        cameraThirdPerson->TranslateRight(-2 * deltaTime);

        glm::vec3 pos = cameraThirdPerson->GetTargetPosition();
        if (CheckWallCollision(pos.x, pos.z, 0.2)) {
            cameraFirstPerson->TranslateRight(2 * deltaTime);
            cameraThirdPerson->TranslateRight(2 * deltaTime);
        }
    }

    if (window->KeyHold(GLFW_KEY_S)) {
        cameraFirstPerson->MoveForward(-2 * deltaTime);
        cameraThirdPerson->MoveForward(-2 * deltaTime);

        glm::vec3 pos = cameraThirdPerson->GetTargetPosition();
        if (CheckWallCollision(pos.x, pos.z, 0.2)) {
            cameraFirstPerson->MoveForward(2 * deltaTime);
            cameraThirdPerson->MoveForward(2 * deltaTime);
        }
    }

    if (window->KeyHold(GLFW_KEY_D)) {
        cameraFirstPerson->TranslateRight(2 * deltaTime);
        cameraThirdPerson->TranslateRight(2 * deltaTime);

        glm::vec3 pos = cameraThirdPerson->GetTargetPosition();
        if (CheckWallCollision(pos.x, pos.z, 0.2)) {
            cameraFirstPerson->TranslateRight(-2 * deltaTime);
            cameraThirdPerson->TranslateRight(-2 * deltaTime);
        }
    }
}

void Tema2::OnKeyPress(int key, int mods)
{
    // switch FPS-TPS
    if (key == GLFW_KEY_X)
    {
        if (isThirdPerson == 0) {
            isThirdPerson = 1;
            camera = cameraThirdPerson;
        }
        else {
            isThirdPerson = 0;
            camera = cameraFirstPerson;
        }
    }
    // intrare in mod atac
    if (key == GLFW_KEY_LEFT_CONTROL) {
        lastPerspective = isThirdPerson;
        isThirdPerson = 0;
        camera = cameraFirstPerson;
        attackMode = true;
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // iesire din mod atac
    if (key == GLFW_KEY_LEFT_CONTROL) {
        isThirdPerson = lastPerspective;
        camera = isThirdPerson ? cameraThirdPerson : cameraFirstPerson;
        attackMode = false;
    }
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    float sensivityOX = 0.001f;
    float sensivityOY = 0.001f;
    cameraThirdPerson->RotateThirdPerson_OX(-sensivityOX * deltaY);
    cameraThirdPerson->RotateThirdPerson_OY(-sensivityOY * deltaX);
    cameraFirstPerson->RotateFirstPerson_OX(-sensivityOX * deltaY);
    cameraFirstPerson->RotateFirstPerson_OY(-sensivityOY * deltaX);
    // valorile de rotatie
    angleCharacter += sensivityOX * deltaX;
    heightCharacter -= sensivityOY * deltaY;
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // lansez proiectil pe click stanga
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        if (attackMode == true) {
            glm::vec3 pos = cameraThirdPerson->GetTargetPosition();
            projectile* newProjectile = new projectile(pos.x, pos.y, pos.z, 1.3, angleCharacter, heightCharacter - 0.15, 0);
            projectilesVector.push_back(*newProjectile);
        }
    }
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}

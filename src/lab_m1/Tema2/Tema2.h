#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/lab_camera.h"
#include "lab_m1/Tema2/transform3D.h"


namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        struct projectile
        {
            projectile() : x(0), y(0), z(0), speed(1.0f), angle(0), height(0), distance(0) {}
            projectile(float x, float y, float z, float speed, float angle, float height, float distance)
                : x(x), y(y), z(z), speed(speed), angle(angle), height(height), distance(distance) {}
            float x;
            float y;
            float z;
            float speed;
            float angle;
            float height;
            float distance;
        };

        struct enemy
        {
            enemy() : x(0), y(0), z(0), speed(0.1f), distance(0), direction(0) {}
            enemy(float x, float y, float z, float speed, float distance, int direction)
                : x(x), y(y), z(z), speed(speed), distance(distance), direction(direction) {}
            float x;
            float y;
            float z;
            float speed;
            float distance;
            int direction;
        };

        struct wall
        {
            wall() : centerX(0), centerZ(0), length(0) {}
            wall(float centerX, float centerZ, float length)
                : centerX(centerX), centerZ(centerZ), length(length) {}
            float centerX;
            float centerZ;
            float length;
        };

    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix) override;
        Mesh* Tema2::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void Tema2::CreateCube(char* name, glm::vec3 color);
        bool Tema2::CheckWallCollision(float newX, float newY, float length);
        bool Tema2::CheckEnemyCollision(float newX, float newY, float length);
        void Tema2::CheckProjectileCollision();
        bool Tema2::PointInArea(float posX, float posY, wall* obstacle);

        void Tema2::DrawEnemies();
        void Tema2::DrawProjectiles();
        void Tema2::DrawPlayer(glm::vec3 pos);
        void Tema2::DrawMaze();

    protected:
        implemented::Camera* camera;
        implemented::Camera* cameraThirdPerson;
        implemented::Camera* cameraFirstPerson;
        glm::mat4 projectionMatrix;
        std::vector<projectile> projectilesVector;
        std::vector<enemy> enemiesVector;
        std::vector<wall> wallsVector;
        float fov;
        float zNear;
        float zFar;
        float distanceToTarget;
        float angleCharacter, heightCharacter;
        int isThirdPerson, lastPerspective;
        bool attackMode;
        float life;

        int mazeDim = 10;
        int maze[10][10] =
        {
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            { 1, 0, 0, 1, 0, 0, 0, 0, 1, 1},
            { 1, 1, 2, 1, 2, 1, 1, 0, 0, 1},
            { 1, 1, 0, 0, 0, 1, 1, 0, 1, 1},
            { 1, 0, 0, 1, 0, 1, 1, 2, 1, 1},
            { 1, 1, 0, 1, 0, 0, 1, 0, 1, 1},
            { 1, 1, 2, 1, 0, 1, 1, 0, 1, 1},
            { 1, 1, 0, 1, 1, 0, 2, 0, 1, 1},
            { 1, 0, 0, 0, 2, 0, 1, 0, 2, 0},
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        };

    };
}

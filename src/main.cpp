#include "raylib.h"
#include <iostream>
#include <memory>
#include <random>

enum Direction { UP, RIGHT, DOWN, LEFT, NONE };

struct SnakeNode {
  Vector2 pos;
  Direction direction;
  std::unique_ptr<SnakeNode> previous;
};

void updateNodePosition(SnakeNode &node) {
  switch (node.direction) {
  case UP:
    node.pos.y--;
    break;
  case RIGHT:
    node.pos.x++;
    break;
  case DOWN:
    node.pos.y++;
    break;
  case LEFT:
    node.pos.x--;
    break;
  case NONE:
    break;
  }
}

Vector2 getRandomFoodPosition(std::uniform_int_distribution<int> &distX,
                              std::uniform_int_distribution<int> &distY,
                              std::mt19937 &gen) {
  return Vector2{(float)distX(gen), (float)distY(gen)};
}

int main() {
  const int SCREEN_WIDTH = 1200;
  const int SCREEN_HEIGHT = 650;
  const int GRID_WIDTH = 25;
  const int GRID_HEIGHT = 15;
  const int CELL_WIDTH = 32;
  const int CELL_HEIGHT = 32;
  const int SCENE_WIDTH = GRID_WIDTH * CELL_WIDTH;
  const int SCENE_HEIGHT = GRID_HEIGHT * CELL_HEIGHT;
  const float SCENE_LEFT = -(SCENE_WIDTH / 2.0f);
  const float SCENE_TOP = -(SCENE_HEIGHT / 2.0f);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distX(0, GRID_WIDTH - 1);
  std::uniform_int_distribution<int> distY(0, GRID_HEIGHT - 1);

  float turnDuration = 0.3f;
  float timer = 0;

  std::unique_ptr<SnakeNode> nextToSpawn = nullptr;

  SnakeNode lastNode =
      SnakeNode{GRID_WIDTH / 2 - 2, GRID_HEIGHT / 2, Direction::RIGHT, nullptr};

  lastNode.previous = std::make_unique<SnakeNode>(SnakeNode{
      GRID_WIDTH / 2 - 1, GRID_HEIGHT / 2, Direction::RIGHT, nullptr});

  lastNode.previous->previous = std::make_unique<SnakeNode>(
      SnakeNode{GRID_WIDTH / 2, GRID_HEIGHT / 2, Direction::RIGHT, nullptr});

  SnakeNode *headNode = lastNode.previous->previous.get();

  Vector2 foodPos = getRandomFoodPosition(distX, distY, gen);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "snake");

  SetTargetFPS(166);

  Camera2D camera;
  camera.offset = Vector2{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
  camera.rotation = 0;
  camera.zoom = 1;

  while (!WindowShouldClose()) {
    // Update

    if (IsKeyPressed(KEY_A) && headNode->direction != Direction::RIGHT) {
      headNode->direction = Direction::LEFT;
    } else if (IsKeyPressed(KEY_D) && headNode->direction != Direction::LEFT) {
      headNode->direction = Direction::RIGHT;
    } else if (IsKeyPressed(KEY_W) && headNode->direction != Direction::DOWN) {
      headNode->direction = Direction::UP;
    } else if (IsKeyPressed(KEY_S) && headNode->direction != Direction::UP) {
      headNode->direction = Direction::DOWN;
    }

    if (timer < turnDuration) {
      timer += GetFrameTime();
    } else {
      updateNodePosition(*headNode);

      if (headNode->pos.x > SCENE_WIDTH || headNode->pos.x < 0 ||
          headNode->pos.y < 0 || headNode->pos.y > SCENE_HEIGHT) {
        std::cout << "FAILED GAME" << std::endl;
      }

      SnakeNode *currentNode = &lastNode;
      while (currentNode->previous) {
        if (headNode->pos.x == currentNode->pos.x &&
            headNode->pos.y == currentNode->pos.y) {
          std::cout << "FAILED GAME" << std::endl;
        }
        updateNodePosition(*currentNode);
        currentNode->direction = currentNode->previous->direction;
        currentNode = currentNode->previous.get();
      }

      if (headNode->pos.x == foodPos.x && headNode->pos.y == foodPos.y) {
        lastNode = SnakeNode{lastNode.pos.x, lastNode.pos.y, Direction::NONE,
                             std::make_unique<SnakeNode>(std::move(lastNode))};
        foodPos = getRandomFoodPosition(distX, distY, gen);
        turnDuration *= 0.9f;
      }

      timer = 0;
    }

    // Drawing
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode2D(camera);

    DrawRectangleLinesEx(
        Rectangle{SCENE_LEFT, SCENE_TOP, SCENE_WIDTH, SCENE_HEIGHT}, 3.0f,
        BLACK);

    SnakeNode *currentNode = &lastNode;
    while (currentNode) {
      DrawRectangle(SCENE_LEFT + currentNode->pos.x * CELL_WIDTH,
                    SCENE_TOP + currentNode->pos.y * CELL_HEIGHT, CELL_WIDTH,
                    CELL_HEIGHT, GREEN);
      currentNode = currentNode->previous.get();
    }

    DrawCircle(SCENE_LEFT + foodPos.x * CELL_WIDTH + CELL_WIDTH / 2,
               SCENE_TOP + foodPos.y * CELL_HEIGHT + CELL_HEIGHT / 2, 10, RED);

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();

  return 0;
}

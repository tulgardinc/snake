#include "raylib.h"
#include <memory>
#include <queue>
#include <random>
#include <string>

enum Direction { UP, RIGHT, DOWN, LEFT, NONE };
enum Screens { GAME, END };

struct SnakeNode {
  Vector2 gridPos;
  Vector2 targetGridPos;
  std::unique_ptr<SnakeNode> previous;
};

Vector2 getNodePos(SnakeNode &node, float normalizedTime) {
  return Vector2{
      (1 - normalizedTime) * node.gridPos.x +
          normalizedTime * node.targetGridPos.x,
      (1 - normalizedTime) * node.gridPos.y +
          normalizedTime * node.targetGridPos.y,
  };
}

void setHeadTarget(SnakeNode &head, Direction dir) {
  switch (dir) {
  case UP:
    head.targetGridPos.y--;
    break;
  case RIGHT:
    head.targetGridPos.x++;
    break;
  case DOWN:
    head.targetGridPos.y++;
    break;
  case LEFT:
    head.targetGridPos.x--;
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

SnakeNode initializeSnake(int gridWidth, int gridHeight) {
  float halfWidth = gridWidth / 2;
  float halfHeight = gridHeight / 2;
  SnakeNode lastNode =
      SnakeNode{halfWidth - 2, halfHeight, halfWidth - 1, halfHeight, nullptr};

  lastNode.previous = std::make_unique<SnakeNode>(
      SnakeNode{halfWidth - 1, halfHeight, halfWidth, halfHeight, nullptr});

  lastNode.previous->previous = std::make_unique<SnakeNode>(
      SnakeNode{halfWidth, halfHeight, halfWidth + 1, halfHeight, nullptr});

  return std::move(lastNode);
}

int WinMain() {
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
  const float INITIAL_TURN_DURATION = 0.20f;
  const float DURATION_RATIO = 0.9f;

  const Vector2 BASE_OFFSET =
      Vector2{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distX(0, GRID_WIDTH - 1);
  std::uniform_int_distribution<int> distY(0, GRID_HEIGHT - 1);

  std::queue<Direction> inputQueue;

  float turnDuration = INITIAL_TURN_DURATION;
  float timer = 0;
  int score = 0;
  Screens currentScreen = Screens::GAME;

  SnakeNode lastNode = initializeSnake(GRID_WIDTH, GRID_HEIGHT);
  SnakeNode *headNode = lastNode.previous->previous.get();

  Vector2 foodPos = getRandomFoodPosition(distX, distY, gen);
  Direction headNodeDirection = Direction::RIGHT;

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "snake");

  SetTargetFPS(166);

  Camera2D camera;
  camera.offset = BASE_OFFSET;
  camera.rotation = 0;
  camera.zoom = 1;

  while (!WindowShouldClose()) {

    if (currentScreen == Screens::GAME) {

      // Update
      if (inputQueue.size() < 2) {
        if (IsKeyPressed(KEY_A)) {
          inputQueue.push(Direction::LEFT);
        } else if (IsKeyPressed(KEY_D)) {
          inputQueue.push(Direction::RIGHT);
        } else if (IsKeyPressed(KEY_W)) {
          inputQueue.push(Direction::UP);
        } else if (IsKeyPressed(KEY_S)) {
          inputQueue.push(Direction::DOWN);
        }
      }

      if (timer < turnDuration) {
        timer += GetFrameTime();
      } else {
        headNode->gridPos = headNode->targetGridPos;

        SnakeNode *currentNode = &lastNode;
        while (currentNode->previous) {
          currentNode->gridPos = currentNode->targetGridPos;
          if (headNode->gridPos.x == currentNode->gridPos.x &&
              headNode->gridPos.y == currentNode->gridPos.y) {
            currentScreen = Screens::END;
          }
          currentNode->targetGridPos = currentNode->previous->targetGridPos;
          currentNode = currentNode->previous.get();
        }

        if (headNode->gridPos.x == foodPos.x &&
            headNode->gridPos.y == foodPos.y) {
          lastNode =
              SnakeNode{lastNode.gridPos.x, lastNode.gridPos.y,
                        lastNode.gridPos.x, lastNode.gridPos.y,
                        std::make_unique<SnakeNode>(std::move(lastNode))};
          foodPos = getRandomFoodPosition(distX, distY, gen);
          score++;
          turnDuration *= DURATION_RATIO;
        }

        if (!inputQueue.empty()) {
          Direction front = inputQueue.front();
          if (!(headNodeDirection == Direction::UP &&
                front == Direction::DOWN) &&
              !(headNodeDirection == Direction::RIGHT &&
                front == Direction::LEFT) &&
              !(headNodeDirection == Direction::DOWN &&
                front == Direction::UP) &&
              !(headNodeDirection == Direction::LEFT &&
                front == Direction::RIGHT)) {
            headNodeDirection = front;
          }
          inputQueue.pop();
        }

        setHeadTarget(*headNode, headNodeDirection);

        if (headNode->targetGridPos.x > GRID_WIDTH - 1 ||
            headNode->targetGridPos.x < 0 || headNode->targetGridPos.y < 0 ||
            headNode->targetGridPos.y > GRID_HEIGHT - 1) {
          currentScreen = Screens::END;
        }

        timer = 0;
      }
    } else {
      if (IsKeyPressed(KEY_R)) {
        turnDuration = INITIAL_TURN_DURATION;
        timer = 0;
        score = 0;

        lastNode = initializeSnake(GRID_WIDTH, GRID_HEIGHT);
        headNode = lastNode.previous->previous.get();
        headNodeDirection = Direction::RIGHT;

        inputQueue = {};
        currentScreen = Screens::GAME;
      }
    }

    // Drawing
    BeginDrawing();

    switch (currentScreen) {
    case GAME:
      ClearBackground(RAYWHITE);
      BeginMode2D(camera);

      for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
          DrawRectangleRounded(Rectangle{SCENE_LEFT + j * CELL_WIDTH + 3,
                                         SCENE_TOP + i * CELL_HEIGHT + 3,
                                         CELL_WIDTH - 6, CELL_HEIGHT - 6},
                               0.3f, 5, Color{242, 242, 242, 255});
        }
      }

      DrawRectangleLinesEx(Rectangle{SCENE_LEFT - 3, SCENE_TOP - 3,
                                     SCENE_WIDTH + 6, SCENE_HEIGHT + 6},
                           3.0f, BLACK);
      {
        SnakeNode *currentNode = &lastNode;
        while (currentNode) {
          Vector2 pos = getNodePos(*currentNode, timer / turnDuration);
          DrawRectangle(SCENE_LEFT + pos.x * CELL_WIDTH,
                        SCENE_TOP + pos.y * CELL_HEIGHT, CELL_WIDTH,
                        CELL_HEIGHT, GREEN);
          currentNode = currentNode->previous.get();
        }
      }

      DrawCircle(SCENE_LEFT + foodPos.x * CELL_WIDTH + CELL_WIDTH / 2,
                 SCENE_TOP + foodPos.y * CELL_HEIGHT + CELL_HEIGHT / 2, 10,
                 RED);

      DrawText(("Score: " + std::to_string(score * 10)).c_str(),
               -SCREEN_WIDTH / 2 + 20, -SCREEN_HEIGHT / 2 + 20, 20, BLACK);
      break;
    case END:

      ClearBackground(BLACK);
      BeginMode2D(camera);

      DrawRectangleLinesEx(Rectangle{SCENE_LEFT - 3, SCENE_TOP - 3,
                                     SCENE_WIDTH + 6, SCENE_HEIGHT + 6},
                           3.0f, RAYWHITE);

      int textWidth =
          MeasureText(("Score: " + std::to_string(score * 10)).c_str(), 30);

      DrawText(("Score: " + std::to_string(score * 10)).c_str(), -textWidth / 2,
               50, 30, RAYWHITE);

      {
        SnakeNode *currentNode = &lastNode;
        while (currentNode) {
          Vector2 pos = getNodePos(*currentNode, timer / turnDuration);
          DrawRectangle(SCENE_LEFT + pos.x * CELL_WIDTH,
                        SCENE_TOP + pos.y * CELL_HEIGHT, CELL_WIDTH,
                        CELL_HEIGHT, RAYWHITE);
          currentNode = currentNode->previous.get();
        }
      }

      int textRestartWidth = MeasureText("R to Restart", 45);

      DrawText("R to Restart", -textRestartWidth / 2, 120, 45, RAYWHITE);
      break;
    }

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();

  return 0;
}

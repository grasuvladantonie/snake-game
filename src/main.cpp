#include <raylib.h>
#include <iostream>
#include <deque>
#include <raymath.h>

using namespace std;

// Paleta noua de culori (Stil Neon/Dark)
Color bgDark = {30, 30, 40, 255};
Color snakeNeon = {0, 228, 255, 255};
Color headColor = {255, 255, 255, 255};
Color snackPink = {255, 51, 153, 255};
Color textYellow = {253, 249, 0, 255};
Color borderBlue = {41, 128, 185, 255};

// Dimensiuni modificate
int tileSize = 28;
int gridDims = 22;
int padding = 60;

// Functie ajutatoare rescrisa
bool IsPointInList(Vector2 point, const deque<Vector2>& list) {
    for (const auto& item : list) {
        if (Vector2Equals(item, point)) {
            return true;
        }
    }
    return false;
}

class Serpent {
public:
    deque<Vector2> body_parts {{6,9}, {5,9}, {4,9}};
    Vector2 move_dir = {1, 0};
    bool pending_growth = false;

    void Render() {
        for (unsigned int i = 0; i < body_parts.size(); i++) {
            float px = body_parts[i].x;
            float py = body_parts[i].y;
            Rectangle segment = {(float)padding + px * tileSize, (float)padding + py * tileSize, (float)tileSize, (float)tileSize};
            
            // Desenam capul diferit de corp pentru un aspect mai finisat
            if (i == 0) {
                DrawRectangleRounded(segment, 0.7, 8, headColor);
            } else {
                DrawRectangleRounded(segment, 0.4, 6, snakeNeon);
            }
        }
    }

    void Move() {
        body_parts.push_front(Vector2Add(body_parts[0], move_dir));
        if (pending_growth) {
            pending_growth = false;
        } else {
            body_parts.pop_back();   
        }
    }

    void Restart() {
        body_parts = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        move_dir = {1, 0};
    }
};

class Snack {
public:
    Vector2 loc;

    Snack(const deque<Vector2>& ignoreList) {
        Spawn(ignoreList);
    }

    void Render() {
        // Am inlocuit textura cu un cerc desenat pentru a evita erorile lipsa fisierelor
        float centerX = padding + loc.x * tileSize + tileSize / 2.0f;
        float centerY = padding + loc.y * tileSize + tileSize / 2.0f;
        DrawCircle(centerX, centerY, tileSize / 2.0f - 2, snackPink);
    }

    Vector2 GetRandomSpot() {
        float rx = GetRandomValue(0, gridDims - 1);
        float ry = GetRandomValue(0, gridDims - 1);
        return Vector2{rx, ry};
    }

    void Spawn(const deque<Vector2>& ignoreList) {
        Vector2 tempLoc = GetRandomSpot();
        while (IsPointInList(tempLoc, ignoreList)) {
            tempLoc = GetRandomSpot();
        }
        loc = tempLoc;
    }
};

class GameEngine {
private:
    double lastTick = 0;
    double speed = 0.18; // Viteza putin modificata (0.2 in original)

public:
    Serpent player = Serpent();
    Snack collectible = Snack(player.body_parts);
    bool isActive = true;
    int currentScore = 0;

    void RenderScene() {
        player.Render();
        collectible.Render();    
    }

    void Tick() {
        double now = GetTime();
        if (now - lastTick >= speed) {
            lastTick = now;
            
            if (isActive) {
                player.Move();
                VerifyEat();
                VerifyBounds();
                VerifyBite();
            }
        }
    }   

    void VerifyEat() {
        if (Vector2Equals(player.body_parts[0], collectible.loc)) {
            collectible.Spawn(player.body_parts);
            player.pending_growth = true;
            currentScore++;
        }
    }

    void VerifyBounds() {
        float headX = player.body_parts[0].x;
        float headY = player.body_parts[0].y;

        if (headX >= gridDims || headX < 0 || headY >= gridDims || headY < 0) {
            TriggerGameOver();
        }
    }

    void VerifyBite() {
        deque<Vector2> bodyWithoutHead = player.body_parts;
        bodyWithoutHead.pop_front();
        if (IsPointInList(player.body_parts[0], bodyWithoutHead)) {
            TriggerGameOver();
        }
    }

    void TriggerGameOver() {
        player.Restart();
        collectible.Spawn(player.body_parts);
        isActive = false;
        currentScore = 0;
    }
};

int main() {
    int windowSize = 2 * padding + tileSize * gridDims;
    InitWindow(windowSize, windowSize, "Neon Serpent");
    SetTargetFPS(60);

    GameEngine engine = GameEngine();

    while (!WindowShouldClose()) {
        // Logica miscarii - am adaugat si tastele WASD pe langa sageti
        if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && engine.player.move_dir.y != 1) {
            engine.player.move_dir = {0, -1};
            engine.isActive = true;
        }
        else if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && engine.player.move_dir.y != -1) {
            engine.player.move_dir = {0, 1};
            engine.isActive = true;
        }
        else if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && engine.player.move_dir.x != 1) {
            engine.player.move_dir = {-1, 0};
            engine.isActive = true;
        }
        else if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && engine.player.move_dir.x != -1) {
            engine.player.move_dir = {1, 0};
            engine.isActive = true;
        }

        // Update-ul jocului
        engine.Tick();

        // Randarea
        BeginDrawing();
        ClearBackground(bgDark);
        
        // Desenare bordura
        Rectangle border = {(float)padding - 5, (float)padding - 5, (float)tileSize * gridDims + 10, (float)tileSize * gridDims + 10};
        DrawRectangleLinesEx(border, 4, borderBlue);
        
        // Desenare text
        DrawText("Neon Serpent", padding - 5, 15, 36, textYellow);
        DrawText(TextFormat("Score: %i", engine.currentScore), padding - 5, padding + tileSize * gridDims + 15, 36, textYellow);
        
        engine.RenderScene();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
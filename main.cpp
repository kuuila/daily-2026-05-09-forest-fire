/**
 * 🌲 Forest Fire — Self-Organized Criticality
 * 
 * Butterfly Effect: A single lightning strike can ignite a catastrophic wildfire.
 * Smallest cause → largest effect.
 * 
 * Rules (2D Cellular Automaton):
 *   EMPTY → TREE   : probability p  (growth)
 *   TREE  → BURN   : probability f  (spontaneous lightning)
 *   TREE  → BURN   : if any neighbor is BURNING (contagion)
 *   BURN  → EMPTY  : deterministic (1 step burn)
 * 
 * Self-Organized Criticality:
 *   System auto-evolves to critical state → power-law fire size distribution
 *   Few large fires + many small fires
 */

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

constexpr int WIDTH  = 120;
constexpr int HEIGHT = 80;
constexpr int CELL   = 8;   // pixel per cell
constexpr float P_GROW = 0.01f;   // empty → tree
constexpr float F_LIGHT = 0.0002f; // tree → burn (lightning)
constexpr int VIEW_SCALE = 1;

enum Cell { EMPTY = 0, TREE = 1, BURN = 2 };

Cell grid[HEIGHT][WIDTH];
Cell next_grid[HEIGHT][WIDTH];

std::mt19937 rng{ std::random_device{}() };
std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };

inline int count_burning_neighbors(int y, int x) {
    int cnt = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue;
            int ny = (y + dy + HEIGHT) % HEIGHT;
            int nx = (x + dx + WIDTH)  % WIDTH;
            if (grid[ny][nx] == BURN) cnt++;
        }
    }
    return cnt;
}

void init() {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            grid[y][x] = (dist(rng) < 0.5f) ? TREE : EMPTY;
}

void step() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            switch (grid[y][x]) {
                case EMPTY:
                    next_grid[y][x] = (dist(rng) < P_GROW) ? TREE : EMPTY;
                    break;
                case TREE: {
                    bool burn_by_neighbor = count_burning_neighbors(y, x) > 0;
                    bool burn_by_lightning = dist(rng) < F_LIGHT;
                    next_grid[y][x] = (burn_by_neighbor || burn_by_lightning) ? BURN : TREE;
                    break;
                }
                case BURN:
                    next_grid[y][x] = EMPTY;
                    break;
            }
        }
    }
    // swap
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            grid[y][x] = next_grid[y][x];
}

// Count largest fire cluster (BFS)
int largest_fire_size() {
    bool visited[HEIGHT][WIDTH] = {};
    int max_sz = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (grid[y][x] == BURN && !visited[y][x]) {
                // BFS
                int sz = 0;
                std::vector<std::pair<int,int>> q;
                q.emplace_back(y, x);
                visited[y][x] = true;
                for (size_t i = 0; i < q.size(); i++) {
                    auto [cy, cx] = q[i];
                    sz++;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dy == 0 && dx == 0) continue;
                            int ny = (cy + dy + HEIGHT) % HEIGHT;
                            int nx = (cx + dx + WIDTH)  % WIDTH;
                            if (grid[ny][nx] == BURN && !visited[ny][nx]) {
                                visited[ny][nx] = true;
                                q.emplace_back(ny, nx);
                            }
                        }
                    }
                }
                if (sz > max_sz) max_sz = sz;
            }
        }
    }
    return max_sz;
}

int tree_count() {
    int cnt = 0;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            if (grid[y][x] == TREE) cnt++;
    return cnt;
}

// Power-law binning for fire size distribution
std::vector<int> fire_history;
int step_count = 0;
int last_fire_step = 0;

void record_fire(int fire_sz) {
    if (fire_sz > 2) { // ignore tiny flickers
        fire_history.push_back(fire_sz);
        if (fire_history.size() > 500) fire_history.erase(fire_history.begin());
    }
}

float estimate_exponent(const std::vector<int>& sizes) {
    if (sizes.size() < 10) return 0;
    // log-log regression: log(N) = -alpha * log(s) + const
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    int n = 0;
    for (int s : sizes) {
        if (s < 3) continue;
        double lx = std::log((double)s);
        double ly = std::log((double)sizes.size()); // frequency proxy
        sum_x += lx; sum_y += ly; sum_xy += lx*ly; sum_xx += lx*lx; n++;
    }
    if (n < 5) return 0;
    double alpha = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    return (float)(-alpha);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH * CELL, HEIGHT * CELL + 80), "🌲 Forest Fire — SOC", sf::Style::Close);
    window.setFramerateLimit(30);

    sf::Font font;
    // Try system fonts
    const char* fonts[] = { "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
                            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
                            "/usr/share/fonts/TTF/DejaVuSansMono.ttf" };
    bool font_loaded = false;
    for (auto f : fonts) {
        if (font.loadFromFile(f)) { font_loaded = true; break; }
    }

    sf::Text text;
    if (font_loaded) {
        text.setFont(font);
        text.setCharacterSize(11);
        text.setFillColor(sf::Color(200, 200, 200));
    }

    sf::RectangleShape cell_rect{ sf::Vector2f((float)CELL - 1, (float)CELL - 1) };

    sf::Color col_empty{ 15, 15, 20 };
    sf::Color col_tree{ 30, 120, 40 };
    sf::Color col_burn{ 255, 80, 10 };

    init();

    int fire_counter = 0;
    int last_max_fire = 0;

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::R) { init(); fire_history.clear(); fire_counter = 0; }
                if (ev.key.code == sf::Keyboard::Space) { step(); }
                if (ev.key.code == sf::Keyboard::Escape) window.close();
            }
            if (ev.type == sf::Event::MouseButtonPressed) {
                // Click to add fire at mouse position
                int mx = ev.mouseButton.x / CELL;
                int my = ev.mouseButton.y / CELL;
                if (my >= 0 && my < HEIGHT && mx >= 0 && mx < WIDTH) {
                    if (grid[my][mx] == TREE) {
                        grid[my][mx] = BURN;
                    }
                }
            }
        }

        for (int i = 0; i < 3; i++) step(); // 3 steps per frame

        int max_fire = largest_fire_size();
        if (max_fire > 2 && max_fire != last_max_fire) {
            record_fire(max_fire);
            fire_counter++;
            last_fire_step = step_count;
        }
        last_max_fire = max_fire;
        step_count++;

        // Draw
        window.clear(col_empty);
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                switch (grid[y][x]) {
                    case TREE: cell_rect.setFillColor(col_tree); break;
                    case BURN: cell_rect.setFillColor(col_burn);  break;
                    default:   cell_rect.setFillColor(col_empty); break;
                }
                cell_rect.setPosition((float)(x * CELL), (float)(y * CELL));
                window.draw(cell_rect);
            }
        }

        // Stats bar
        sf::RectangleShape bar{ sf::Vector2f((float)(WIDTH * CELL), 76.f) };
        bar.setFillColor(sf::Color(10, 10, 15));
        bar.setPosition(0, (float)(HEIGHT * CELL));
        window.draw(bar);

        if (font_loaded) {
            int trees = tree_count();
            float coverage = trees * 100.0f / (WIDTH * HEIGHT);
            float alpha = estimate_exponent(fire_history);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);
            oss << "Step:" << std::setw(8) << step_count
                << "  Trees:" << std::setw(6) << trees << "(" << coverage << "% coverage)"
                << "  Largest fire:" << std::setw(6) << last_max_fire
                << "  Fires:" << fire_counter
                << "  ~alpha:" << std::setprecision(2) << alpha;
            text.setString(oss.str());
            text.setPosition(6, (float)(HEIGHT * CELL + 4));
            window.draw(text);

            std::ostringstream h2;
            h2 << "R=Reset  Space=Step  Click=Ignite  Esc=Quit";
            text.setString(h2.str());
            text.setPosition(6, (float)(HEIGHT * CELL + 52));
            window.draw(text);
        }

        window.display();
    }

    std::cout << "\n=== Forest Fire Simulation Complete ===\n";
    std::cout << "Total steps: " << step_count << "\n";
    std::cout << "Total fires: " << fire_counter << "\n";
    if (!fire_history.empty()) {
        int max_sz = 0;
        for (int s : fire_history) if (s > max_sz) max_sz = s;
        std::cout << "Largest fire: " << max_sz << " cells\n";
        std::cout << "~Power-law exponent alpha: " << std::fixed << std::setprecision(2)
                  << estimate_exponent(fire_history) << "\n";
        std::cout << "(Alpha ≈ 1.0-2.0 for forest fire SOC)\n";
    }
    return 0;
}

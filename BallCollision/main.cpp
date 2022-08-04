#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include "MoreMath.h"

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 200;
constexpr float PI = 3.1415f;

Math::MiddleAverageFilter<float,100> fpscounter;

struct Ball
{
    sf::Vector2f p;
    sf::Vector2f dir;
    float r = 0;
    float speed = 0;
};

struct Cell
{
    bool isEmpty = true;
    std::vector<Ball*> objects;
};

struct Grid
{
    int rows, columns;
    int size;
    sf::Vector2i cellSize;
    Cell* cells;

    Grid(int r, int c)
    {
        rows = r;
        columns = c;
        size = r * c;
        cellSize.x = WINDOW_X / columns;
        cellSize.y = WINDOW_Y / rows;
        cells = new Cell[size];
    }

    ~Grid()
    {
        delete[] cells;
    }

    void Insert(Ball* b)
    {
        int _x = b->p.x / cellSize.x;
        int _y = b->p.y / cellSize.y;

        int indx = _y * columns + _x;
        if (indx < rows * columns)
        {
            cells[indx].objects.push_back(b);
            cells[indx].isEmpty = false;
        }
    }

    void Reset()
    {
        for (int i = 0; i < size; ++i)
        {
            cells[i].isEmpty = true;
            cells[i].objects.clear();
        }
    }
};

inline void move_ball(Ball& ball, float deltaTime)
{
    float dx = ball.dir.x * ball.speed * deltaTime;
    float dy = ball.dir.y * ball.speed * deltaTime;
    ball.p.x += dx;
    ball.p.y += dy;
}

void check_bounds(Ball& b)
{
    if (b.p.x - b.r < 0 || b.p.x + b.r > WINDOW_X)
    {
        b.dir = reflected(b.dir, sf::Vector2f(1, 0));
    }

    if (b.p.y - b.r < 0 || b.p.y + b.r > WINDOW_Y)
    {
        b.dir = reflected(b.dir, sf::Vector2f(0, 1));
    }
}

inline void check_collision(Ball& b1, Ball& b2)
{
    sf::Vector2f distance = b2.p - b1.p;
    float sqMag = sqr_magnitude(distance);
    float radSum = b2.r + b1.r; 
    radSum *= radSum;

    if (sqMag <= radSum)
    {
        sf::Vector2f clsnNormal = normalized(distance);

        float m1 = b1.r;
        float m2 = b2.r;

        //base balls velocities
        sf::Vector2f i1 = b1.dir * b1.speed;
        sf::Vector2f i2 = b2.dir * b2.speed;

        //velocities projections
        float p1 = dot(i1, clsnNormal);
        float p2 = dot(i2, clsnNormal);

        float v1 = ((m1 - m2) * p1 + 2 * m2 * p2) / (m1 + m2);
        float v2 = ((m2 - m1) * p2 + 2 * m1 * p1) / (m1 + m2);

        i1 += (v1 - p1) * clsnNormal;
        i2 += (v2 - p2) * clsnNormal;

        //penetration check for "sticked" balls
        float pntr = (radSum - sqMag);
        if (pntr > b1.r * b1.r)
            b1.p -= clsnNormal * b1.r;

        b1.dir = normalized(i1);
        b2.dir = normalized(i2);
        b1.speed = magnitude(i1);
        b2.speed = magnitude(i2);
    }
}

inline void draw_balls(sf::RenderWindow& window, std::vector<Ball>& balls, int slices = 16)
{
    //using array draw because of too big overhead on shapes drawing
    sf::VertexArray vertices(sf::Points, balls.size() * slices); 

    for (int i = 0; i < balls.size(); i += 1)
    {
        auto p = balls[i].p;
        auto r = balls[i].r;

        sf::Vector2f v = { 0, -1 };
        float a = 0;
        for (size_t j = 0; j < slices; j++)
        {
            vertices[i * slices + j] = p + rotated(v, a) * r;
            a += 2 * PI / (float)slices;
        }
    }
    window.draw(vertices);
}

void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    std::string string(c);
    sf::String str(c);
    window.setTitle(str);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    srand(time(NULL));

    std::vector<Ball> balls;
    Grid grid(8, 8);

    // randomly initialize balls
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
    {
        Ball newBall;
        newBall.p.x = rand() % WINDOW_X;
        newBall.p.y = rand() % WINDOW_Y;
        newBall.dir.x = (-5 + (rand() % 10)) / 3.;
        newBall.dir.y = (-5 + (rand() % 10)) / 3.;
        newBall.r = 5 + rand() % 5;
        newBall.speed = 30 + rand() % 30;
        balls.push_back(newBall);
    }

    sf::Clock clock;
    float lastime = clock.restart().asSeconds();

    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;
        
        //spatial grid for more optimal collision checks
        grid.Reset();
        for (auto& ball : balls)
        {
            grid.Insert(&ball);
        }

        for (auto i = 0; i < grid.size; ++i)
        {
            Cell& cell = grid.cells[i];

            if (!cell.isEmpty)
            {
                for (size_t j = 0; j < cell.objects.size(); j++)
                {
                    check_bounds(*cell.objects[j]);
                    for (size_t k = j+1; k < cell.objects.size(); k++)
                    {
                        check_collision(*cell.objects[j], *cell.objects[k]);
                    }
                }
            }
        }

        for (auto& ball : balls)
        {
            move_ball(ball, deltaTime);
        }

        window.clear();
        draw_balls(window, balls);

		draw_fps(window, fpscounter.getAverage());
		window.display();
    }
    return 0;
}

// Snake game

int WIDTH = 40;
int HEIGHT = 20;

int x, y, fruitX, fruitY, score, gameover;
int tailX[100], tailY[100];
int nTail;

int STOP = 0, LEFT = 1, RIGHT = 2, UP = 3, DOWN = 4;
int dir; //direction of the snake

void Setup()
{
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    gameover = 0;
    dir = STOP;
    x = WIDTH / 2;
    y = HEIGHT / 2;
    fruitX = rand() % WIDTH;
    fruitY = rand() % HEIGHT;
    score = 0;
}

void Draw()
{
    int i, j, k;
    clear();
    i = 0;
    while (i < WIDTH + 2) {
        printw("#");
        i = i + 1;
    }
    printw("\n");

    i = 0;
    while (i < HEIGHT) {
        j = 0;
        while (j < WIDTH) {
            if (j == 0)
                printw("#");
            if (i == y && j == x)
                printw("O");
            else if (i == fruitY && j == fruitX)
                printw("X");
            else {
                int print = 0;
                k = 0;
                while (k < nTail) {
                    if (tailX[k] == j && tailY[k] == i) {
                        printw("o");
                        print = 1;
                    }
                    k = k + 1;
                }
                if (print == 0)
                    printw(" ");
            }

            if (j == WIDTH - 1)
                printw("#");
            j = j + 1;
        }
        printw("\n");
        i = i + 1;
    }

    i = 0;
    while (i < WIDTH + 2) {
        printw("#");
        i = i + 1;
    }
    printw("\n");
    printw("Score: %d\n", score);
    printw("A=Left, W=Up, S=Down, D=Right. X to exit\n");
    refresh();
}

void Input()
{
    int ch = getch();
    if (ch == 97) // 'a' move left
        dir = LEFT;
    else if (ch == 100) //'d' move right
        dir = RIGHT;
    else if (ch == 119) //'w' move up
        dir = UP;
    else if (ch == 115) // 's' move down
        dir = DOWN;
    else if (ch == 120) //'x' to exit
        gameover = 1;
    else
        dir = STOP;
}

void Logic()
{
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    int i = 1;
    tailX[0] = x;
    tailY[0] = y;
    while (i < nTail) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
        i = i + 1;
    }

    if (dir == LEFT) {
        x = x - 1;
    } else if (dir == RIGHT) {
        x = x + 1;
    } else if (dir == UP) {
        y = y - 1;
    } else if (dir == DOWN) {
        y = y + 1;
    }

    if (x > WIDTH - 1)
        x = 0;
    else if (x < 0)
        x = WIDTH - 1;

    if (y > HEIGHT - 1)
        y = 0;
    else if (y < 0)
        y = HEIGHT - 1;

    i = 0;
    while (i < nTail) {
        if (tailX[i] == x && tailY[i] == y)
            gameover = 1;
        i = i + 1;
    }

    if (x == fruitX && y == fruitY) {
        score = score + 10;
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
        nTail = nTail + 1;
    }
}

int main()
{
    Setup();
    while (gameover == 0) {
        Draw();
        Input();
        Logic();
        usleep(50000);
    }
    endwin();
    printf("Game over. You scored %d\n", score);
    return 0;
}


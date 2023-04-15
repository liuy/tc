// Plane game

int WIDTH=40;
int HEIGHT=20;
int BULLET_NUM=100;
int ENEMY_NUM=10;

int x, y, score, gameover;
int bulletX[100], bulletY[100], bulletActive[100];
int enemyX[10], enemyY[10], enemyActive[10];

void Setup()
{
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    gameover = 0;
    x = WIDTH / 2;
    y = HEIGHT - 2;
    score = 0;
    int i = 0;
    while (i < BULLET_NUM) {
        bulletX[i] = 1-2;
        bulletY[i] = 1-2;
        bulletActive[i] = 0;
        i = i + 1;
    }
    i = 0;
    while (i < ENEMY_NUM) {
        enemyX[i] = rand() % WIDTH;
        enemyY[i] = rand() % 3;
        enemyActive[i] = 1;
        i = i + 1;
    }
}

void Draw()
{
    int i = 0, j = 0;
    clear();
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
                printw("A");
            else {
                int k = 0;
                int print = 0;
                while (k < BULLET_NUM) {
                    if (bulletX[k] == j && bulletY[k] == i && bulletActive[k] == 1) {
                        printw("|");
                        print = 1;
                    }
                    k = k + 1;
                }
                k = 0;
                while (k < ENEMY_NUM) {
                    if (enemyX[k] == j && enemyY[k] == i && enemyActive[k] == 1) {
                        printw("U");
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
    printw("A=Left, D=Right, Space=Attack, X=Exit");
    refresh();
}

void Input()
{
    int ch = getch();
    int ibreak = 0;
    if (ch == 97) // 'a'
        x = x - 1;
    else if (ch == 100)//'d'
        x = x + 1;
    else if (ch == 32) {// ' '
        int i = 0;
        while (i < BULLET_NUM && ibreak == 0) {
            if (bulletActive[i] == 0) {
                bulletActive[i] = 1;
                bulletX[i] = x;
                bulletY[i] = y - 1;
                ibreak = 1;
            }
            i = i + 1;
        }
    } else if (ch == 120) // 'x'
        gameover = 1;
}

void Logic()
{
    int i = 0, j = 0;
    while (i < BULLET_NUM) {
        if (bulletActive[i] == 1) {
            bulletY[i] = bulletY[i] - 1;
            if (bulletY[i] < 0) {
                bulletActive[i] = 0;
            }
        }
        i = i + 1;
    }
    i = 0;
    while (i < ENEMY_NUM) {
        if (enemyActive[i] == 1) {
            enemyY[i] = enemyY[i] + 1;
            if (enemyY[i] >= HEIGHT) {
                enemyActive[i] = 0;
            }
            j = 0;
            while (j < BULLET_NUM) {
                if (bulletActive[j] == 1 && bulletX[j] == enemyX[i] && bulletY[j] == enemyY[i]) {
                    bulletActive[j] = 0;
                    enemyActive[i] = 0;
                    score = score + 10;
                }
                j = j + 1;
            }
            if (enemyX[i] == x && enemyY[i] == y) {
                gameover = 1;
            }
        }
        i = i + 1;
    }
    i = 0;
    while (i < BULLET_NUM) {
        j = 0;
        while (j < ENEMY_NUM) {
            if (bulletActive[i] == 1 && enemyActive[j] == 1 && bulletX[i] == enemyX[j] && bulletY[i] == enemyY[j]) {
                bulletActive[i] = 0;
                enemyActive[j] = 0;
                score = score + 10;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    if (x <= 0) {
        x = 0;
    }
    if (x >= WIDTH - 1) {
        x = WIDTH - 1;
    }
}

int main()
{
    Setup();
    while (gameover == 0) {
        Draw();
        Input();
        Logic();
        usleep(5000);
        int i = 0, ibreak = 0;
        while (i < ENEMY_NUM && ibreak == 0) {
            if (enemyActive[i] == 0) {
                enemyActive[i] = 1;
                enemyX[i] = rand() % WIDTH;
                enemyY[i] = rand() % 3;
                ibreak = 1;
            }
            i = i + 1;
        }
    }
    endwin();
    printf("Game over. You scored %d\n", score);
    return 0;
}


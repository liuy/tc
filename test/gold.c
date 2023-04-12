int main()
{
    int i = 4, j, found;
    // Goldbach Conjecture Equations
    while (i <= 500) {
        j = 2;
        found = 0;
        while (j <= i / 2 && found == 0) {
            if (is_prime(j) && is_prime(i-j)) {
                printf("[%d = %d + %d] ", i, j, i-j);
                found = 1;
            }
            j = j + 1;
        }
        i = i + 2;
    }
    printf("\n");
    return 0;
}

int is_prime(int num) {
    if (num < 2) {
        return 0;
    }
    int i = 2;
    while (i * i <= num) {
        if (num % i == 0) {
            return 0;
        }
        i = i + 1;
    }
    return 1;
}


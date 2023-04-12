int main() {
    int i = 3;
    // print twin primes less than 10000
    while (i < 10000) {
        if (is_prime(i) && is_prime(i + 2)) {
            printf("(%d, %d) ", i, i + 2);
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


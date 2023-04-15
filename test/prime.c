// Print prime numbers within 1000
int main()
{
    int count = 0;
    int num = 2;
    while (count < 1000) {
        if (is_prime(num)) {
            printf("%d ", num);
            count = count + 1;
        }
        num = num + 1;
    }
    printf("\n");
    return 0;
}

int is_prime(int num)
{
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

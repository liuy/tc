// Print 42 Fibonacci numbers
int main()
{
    int n = 42;
    int i = 1;
    while (i <= n) {
        i = i + 1;
        printf("%d ", fibonacci(i));
    }
    printf("\n");
    return 0;
}

int fibonacci(int n) {
    if (n <= 2) {
        return 1;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

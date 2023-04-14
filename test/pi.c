// compute the first 800 digits of pi
int main()
{
    int r[2801];
    int i, k;
    int b, d;
    int c = 0;
    int done;

    i = 0;
    while (i < 2800) {
        r[i] = 2000;
        i = i + 1;
    }

    k = 2800;
    while (k > 0) {
        d = 0;
        done = 0;

        i = k;
        while (done == 0) {
            d = d + r[i] * 10000;
            b = 2 * i - 1;

            r[i] = d % b;
            d = d / b;
            i = i - 1;
            if (i == 0) {
                done = 1;
            } else {
                d = d * i;
            }
        }
        printf("%.4d", c + d / 10000);
        c = d % 10000;

        k = k - 14;
    }

    printf("\n");
    return 0;
}


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int read_binary(const char *fname, char *buf, size_t buf_size, size_t n)
{
    FILE *f = NULL;
    int ret;

    f = fopen(fname, "rb");
    ret = fread(buf, 1, n, f);
    if (ret != n) {
        fprintf(stderr, "read '%s' error: ret is %d.\n", fname, ret);
        return -1;
    }
    fclose(f);

    return ret;
}

static int write_binary(const char *fname, char *buf, size_t buf_size, size_t n)
{
    FILE *f = NULL;
    int ret;

    f = fopen(fname, "wb+");
    ret = fwrite(buf, 1, n, f);
    if (ret != n) {
        fprintf(stderr, "write '%s' error: ret is %d.\n", fname, ret);
        return -1;
    }
    fclose(f);

    return ret;
}

static int resize_binary(char *ifname, char *ofname, int n, int isbootsect)
{
    struct stat st;
    char *buf;
    int buf_size = n;
    int ret;

    if (stat(ifname, &st) != 0) {
        fprintf(
            stderr, "Error opening file '%s': %s\n", ifname, strerror(errno));
        return -1;
    }

    printf("'%s' size: %ld bytes\n", ifname, st.st_size);
    if (st.st_size > (isbootsect ? 510 : n)) {
        fprintf(stderr, "%ld > %d\n", st.st_size, n);
        return -1;
    }

    if (isbootsect)
        buf_size = 512;
    buf = (char *)malloc(buf_size * sizeof(char));
    memset(buf, 0, buf_size);

    ret = read_binary(ifname, buf, buf_size, st.st_size);
    if (ret < 0)
        return -1;

    if (isbootsect) {
        buf[510] = 0x55;
        buf[511] = 0xAA;
    }

    ret = write_binary(ofname, buf, buf_size, buf_size);
    if (ret < 0)
        return -1;

    return 0;
}

static void help()
{
    printf("Usage:\n");
    printf("\tsign -i inputfile -o outputfile -n size -b(optional)\n");
    printf("\n");
    printf("Options:\n");
    printf("\t-i\t input file\n");
    printf("\t-o\t output file\n");
    printf("\t-n\t output file size\n");
    printf("\t-b\t set for boot sect\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    char *ifname = NULL;
    char *ofname = NULL;
    int c, ret;
    int isbootsect = 0;
    int size = 512;

    opterr = 0;

    while ((c = getopt(argc, argv, "i:o:bn:")) != -1) {
        switch (c) {
        case 'i':
            printf("i %s\n", optarg);
            ifname = optarg;
            break;
        case 'o':
            ofname = optarg;
            break;
        case 'b':
            isbootsect = 1;
            break;
        case 'n':
            size = atoi(optarg);
            break;
        default:
            fprintf(stderr, "error parsing arguments %c\n", c);
            goto err;
            break;
        }
    }

    if (!ifname || !ofname || size <= 0) {
        fprintf(stderr, "error input options:\n");
        fprintf(stderr, "\tinput file %s, output file %s, size %d\n", ifname,
            ofname, size);
        goto err;
    }

    ret = resize_binary(ifname, ofname, size, isbootsect);
    if (ret < 0) {
        fprintf(stderr, "error resize binary file\n");
        goto err;
    }

    printf("resize %s to %d bytes %s success!\n", ifname, size, ofname);
    return 0;
err:
    help();
    return -1;
}
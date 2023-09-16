#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *path;
    int pid;

    pid = fork();
    if (pid < 0)
        return pid;

    if (!pid) {
        path = argv[1];
        return execv(path, &argv[2]);
    }

    return 0;
}

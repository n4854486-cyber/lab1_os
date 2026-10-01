#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

int main(void) {

    srand(time(NULL));

    const char msg[] = "Write filenames for the first and the second file:\n";
	write(STDERR_FILENO, msg, sizeof(msg));

    char file1[256], file2[256];
    //считываем название первого файла и именуем файл для первого процесса
    size_t bytes1 = read(STDIN_FILENO, file1, 255);
    if (bytes1 < 0){
        const char msg[] = "error: wrong filename\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    file1[bytes1] = '\0';
    for(size_t i = 0; i < bytes1; i++){
        if (file1[i] == '\n' || file1[i] == '\r'){
            file1[i] = '\0';
            break;
        }
    }
    //считываем название второго файла и именуем файл для второго процесса
    size_t bytes2 = read(STDIN_FILENO, file2, 255);
    if (bytes2 < 0){
        const char msg[] = "error: wrong filename\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    file2[bytes2] = '\0';
    for(size_t i = 0; i < bytes2; i++){
        if (file2[i] == '\n' || file2[i] == '\r'){
            file2[i] = '\0';
            break;
        }
    }
    //ввод строчек до Ctrl+D
    const char entering[] = "Enter the lines\n";
    write(STDERR_FILENO, entering, sizeof(entering) - 1);
     //настройка направлений ввода-вывода "труб"
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    //создаем первый процесс
    const pid_t child1 = fork();
    if (child1 == -1){
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    if (child1 == 0){
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        int fd1 = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd1 == -1) {
            const char msg[] = "error: open file1\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        dup2(pipe1[0], STDIN_FILENO);
        dup2(fd1, STDOUT_FILENO);
        close(pipe1[0]);
        close(fd1);
        char *args[] = {"child", NULL};
        char *env[] = {NULL};

        execve("./child", args, env);

        const char msg[] = "error: execve failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    //создаем второй процесс
    const pid_t child2 = fork();
    if (child2 == -1){
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    if (child2 == 0){
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);
        int fd2 = open(file2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd2 == -1) {
            const char msg[] = "error: open file2\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        dup2(pipe2[0], STDIN_FILENO);
        dup2(fd2, STDOUT_FILENO);
        close(pipe2[0]);
        close(fd2);

        char *args[] = {"child", NULL};
        char *env[] = {NULL};

        execve("./child", args, env);

        const char msg[] = "error: execve failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    close(pipe1[0]);
    close(pipe2[0]);
    //распределяем строки
    char buf[4098];
    size_t i = 0;
    char c;
    while(read(STDIN_FILENO, &c, 1) == 1){
        if(c == '\n' || i >= sizeof(buf)) {
            if (i == 0){
                break;
            }
            if (rand() % 100 < 80) {
                write(pipe1[1], buf, i);
                write(pipe1[1], "\n", 1);
            }else {
                write(pipe2[1], buf, i);
                write(pipe2[1], "\n", 1);
            }
            i = 0;
        } else {
            if(i < sizeof(buf) - 1) {
                buf[i] = c;
                i++;
            }
        }   
    }
    close(pipe1[1]);
    close(pipe2[1]);
    //отправляем процессы в ожидание
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    // выводим содержимое файлолв
    const char header1[] = "\n===== Содержимое файла 1 =====\n";
    write(STDOUT_FILENO, header1, sizeof(header1) - 1);

    int fd = open(file1, 0);
    if (fd != -1) {
        char buf[4096];
        int bytes_read;
        while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes_read);
        }
        close(fd);
    } else {
        write(STDERR_FILENO, "Не удалось открыть файл 1\n", 26);
    }

    const char header2[] = "\n===== Содержимое файла 2 =====\n";
    write(STDOUT_FILENO, header2, sizeof(header2) - 1);

    fd = open(file2, 0);
    if (fd != -1) {
        char buf[4096];
        int bytes_read;
        while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes_read);
        }
        close(fd);
    } else {
        write(STDERR_FILENO, "Не удалось открыть файл 2\n", 26);
    }
    return 0;
}
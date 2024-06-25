#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>

#define SUCCESS 0
#define ERROR 1
#define BUFFER_SIZE 1024
#define BUFFER_USED_LIMIT 512

FILE *log_out = NULL;
int canWrite = ERROR;
char logBuffer[BUFFER_SIZE];
int bufferWritten = 0;

bool logging = true;

pthread_mutex_t logger_mutex;
time_t timer;
struct tm* tm_info;

int init_logger(char *logLocation) {
    log_out = fopen(logLocation, "a");
    canWrite = (log_out == NULL) ? ERROR : SUCCESS;
    return canWrite;
}

void dump_logger_buffer(void) {
    if (canWrite == SUCCESS) {
        logBuffer[bufferWritten] = '\0';
        fprintf(log_out, "%s", logBuffer);
        bufferWritten = 0;
        fflush(log_out);
    }
}

void update_logger_content(void) {
    pthread_mutex_lock(&logger_mutex);
    dump_logger_buffer();
    pthread_mutex_unlock(&logger_mutex);
}

void close_logger(void) {
    if (canWrite == SUCCESS) {
        pthread_mutex_lock(&logger_mutex);
        dump_logger_buffer();
        fclose(log_out);
        // no se desbloquea el mutex para que no escriba en algo no valido
    }
}

size_t count_format_specifiers(const char *str) {
    size_t count = 0;
    while (*str) {
        if (*str == '%' && *(str + 1) != '%') {
            count++;
            str++;
        }
        str++;
    }
    return count;
}

int log_data(char *str, ...) {
    if (canWrite != SUCCESS)
        return ERROR;
    if (!logging)
        return SUCCESS;

    // While not very efficient, this stops invalid memory from being accessed
    va_list args_aux;
    va_start(args_aux, str);
    size_t argc = 0;
    while (va_arg(args_aux, void*) != NULL) {
        argc++;
    }
    if ((argc - 1) < count_format_specifiers(str))
        return ERROR;

    pthread_mutex_lock(&logger_mutex);

    va_list args;
    va_start(args, str);

    if (bufferWritten >= BUFFER_USED_LIMIT)
        dump_logger_buffer();

    char buffer[32];
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S : ", tm_info);

    bufferWritten += sprintf(logBuffer + bufferWritten, "%s", buffer);

    int remainingBuffer = BUFFER_SIZE - bufferWritten - 1;
    int written = vsnprintf(logBuffer + bufferWritten, remainingBuffer, str, args);
    bufferWritten += written;
    logBuffer[bufferWritten++] = '\n';

    if (bufferWritten >= BUFFER_USED_LIMIT) {
        dump_logger_buffer();
    }

    va_end(args);

    pthread_mutex_unlock(&logger_mutex);
    return SUCCESS;
}

void set_logging(bool value) {
    logging = value;
}

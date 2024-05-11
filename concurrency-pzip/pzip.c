// pzip.c, JN, 08.05.2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#define MAX_THREADS 16

typedef struct {
    char* data;
    size_t size;
} chunk_t;

typedef struct {
    chunk_t *chunks;
    int num_chunks;
    char **output;   // Storage for output from each thread
    size_t *lengths; // Length of each thread's output
    int thread_count;
} thread_data_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int current_chunk = 0;

void compress_and_store(chunk_t chunk, char **buffer, size_t *length) {
    size_t buffer_size = chunk.size * 2;
    *buffer = malloc(buffer_size); // Allocate initial buffer
    *length = 0;

    if (chunk.size == 0) return; // Return early if chunk is empty

    char prev_char = chunk.data[0];
    int count = 1;

    for (size_t i = 1; i < chunk.size; i++) {
        if (chunk.data[i] == prev_char) {
            count++;
        } else {
            if (*length + sizeof(int) + sizeof(char) > buffer_size) {
                buffer_size *= 2;
                *buffer = realloc(*buffer, buffer_size); // Resize buffer as needed
            }
            memcpy(*buffer + *length, &count, sizeof(int));
            *length += sizeof(int);
            memcpy(*buffer + *length, &prev_char, sizeof(char));
            *length += sizeof(char);

            prev_char = chunk.data[i];
            count = 1;
        }
    }
    if (*length + sizeof(int) + sizeof(char) > buffer_size) {
        buffer_size += sizeof(int) + sizeof(char);
        *buffer = realloc(*buffer, buffer_size); // Final resize for the last sequence
    }
    memcpy(*buffer + *length, &count, sizeof(int));
    *length += sizeof(int);
    memcpy(*buffer + *length, &prev_char, sizeof(char));
    *length += sizeof(char);
}

void *compress_chunk(void *arg) {
    thread_data_t *thread_data = (thread_data_t *)arg;
    int local_chunk;

    while (1) {
        pthread_mutex_lock(&mutex);
        local_chunk = current_chunk;
        if (current_chunk >= thread_data->num_chunks) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        current_chunk++;
        pthread_mutex_unlock(&mutex);

        compress_and_store(thread_data->chunks[local_chunk],
                           &thread_data->output[local_chunk],
                           &thread_data->lengths[local_chunk]);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    }

    int num_threads = get_nprocs();
    if (num_threads > MAX_THREADS) {
        num_threads = MAX_THREADS;
    }

    thread_data_t thread_data;
    thread_data.num_chunks = argc - 1;
    thread_data.chunks = malloc(thread_data.num_chunks * sizeof(chunk_t));
    thread_data.output = calloc(thread_data.num_chunks, sizeof(char*));
    thread_data.lengths = calloc(thread_data.num_chunks, sizeof(size_t));
    thread_data.thread_count = num_threads;

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (!file) {
            fprintf(stderr, "Error opening file: %s\n", argv[i]);
            exit(1);
        }

        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *data = malloc(file_size);
        if (data == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            fclose(file); // Always remember to close the file
            exit(1);
        }

        size_t bytes_read = fread(data, 1, file_size, file);
        if (bytes_read != file_size) {
            fprintf(stderr, "Error reading file: %s\n", argv[i]);
            free(data);
            fclose(file);
            exit(1);
        }
        fclose(file);

        thread_data.chunks[i - 1].data = data;
        thread_data.chunks[i - 1].size = file_size;
    }

    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, compress_chunk, &thread_data);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Sequentially write all compressed data to stdout
    for (int i = 0; i < thread_data.num_chunks; i++) {
        fwrite(thread_data.output[i], 1, thread_data.lengths[i], stdout);
        free(thread_data.output[i]);
    }

    // Cleanup all allocated memory
    for (int i = 0; i < thread_data.num_chunks; i++) {
        free(thread_data.chunks[i].data);
    }
    free(thread_data.chunks);
    free(thread_data.output);
    free(thread_data.lengths);

    return 0;
}

// eof

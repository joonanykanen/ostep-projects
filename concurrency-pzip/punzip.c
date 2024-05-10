// punzip.c, JN, 10.05.2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#define MAX_THREADS 16

typedef struct {
    char *data;
    size_t size;
} chunk_t;

typedef struct {
    chunk_t *chunks;
    int num_chunks;
} thread_data_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int current_chunk = 0;

void *decompress_chunk(void *arg) {
    thread_data_t *thread_data = (thread_data_t *)arg;
    chunk_t *decompressed_chunks = malloc(thread_data->num_chunks * sizeof(chunk_t));
    int decompressed_chunk_index = 0;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (current_chunk >= thread_data->num_chunks) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        int chunk_index = current_chunk++;
        pthread_mutex_unlock(&mutex);

        chunk_t chunk = thread_data->chunks[chunk_index];
        size_t decompressed_size = 0;

        // Calculate the decompressed size
        size_t i = 0;
        while (i < chunk.size) {
            int count;
            memcpy(&count, chunk.data + i, sizeof(int));
            i += sizeof(int) + sizeof(char);
            decompressed_size += count;
        }

        char *decompressed_data = malloc(decompressed_size * sizeof(char));
        size_t current_size = 0;

        i = 0;
        while (i < chunk.size) {
            int count;
            char character;

            // Read the count and character from the compressed data
            memcpy(&count, chunk.data + i, sizeof(int));
            i += sizeof(int);
            memcpy(&character, chunk.data + i, sizeof(char));
            i += sizeof(char);

            // Write the decompressed data to the buffer
            for (int j = 0; j < count; j++) {
                decompressed_data[current_size++] = character;
            }
        }

        decompressed_chunks[decompressed_chunk_index].data = decompressed_data;
        decompressed_chunks[decompressed_chunk_index].size = decompressed_size;
        decompressed_chunk_index++;
    }

    // Write the decompressed chunks to the output file in the correct order
    for (int i = 0; i < decompressed_chunk_index; i++) {
        fwrite(decompressed_chunks[i].data, sizeof(char), decompressed_chunks[i].size, stdout);
        free(decompressed_chunks[i].data);
    }

    free(decompressed_chunks);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "punzip: file1 [file2 ...]\n");
        exit(1);
    }

    // Determine the number of threads to create based on the number of CPU resources
    int num_threads = get_nprocs();
    if (num_threads > MAX_THREADS) {
        num_threads = MAX_THREADS;
    }

    thread_data_t thread_data;
    thread_data.num_chunks = argc - 1;
    thread_data.chunks = malloc(thread_data.num_chunks * sizeof(chunk_t));

    // Read the compressed files into memory
    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "rb");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", argv[i]);
            exit(1);
        }

        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *data = malloc(file_size * sizeof(char));
        size_t bytes_read = fread(data, sizeof(char), file_size, file);
        if (bytes_read != file_size) {
            fprintf(stderr, "Error reading file: %s\n", argv[i]);
            free(data);
            exit(1);
        }
        fclose(file);

        thread_data.chunks[i - 1].data = data;
        thread_data.chunks[i - 1].size = file_size;
    }

    // Create threads to decompress the chunks
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, decompress_chunk, &thread_data);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Free the allocated memory
    for (int i = 0; i < thread_data.num_chunks; i++) {
        free(thread_data.chunks[i].data);
    }
    free(thread_data.chunks);

    return 0;
}

// eof

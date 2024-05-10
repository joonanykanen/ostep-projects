// pzip.c, JN, 08.05.2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#define MAX_THREADS 16

// Structure to represent a chunk of data
typedef struct {
    char *data;
    size_t size;
} chunk_t;

// Structure to hold the chunks of data for each thread
typedef struct {
    chunk_t *chunks;
    int num_chunks;
} thread_data_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int current_chunk = 0;

// Function executed by each thread to compress a chunk of data
void *compress_chunk(void *arg) {
    thread_data_t *thread_data = (thread_data_t *)arg;
    chunk_t *compressed_chunks = malloc(thread_data->num_chunks * sizeof(chunk_t));
    int compressed_chunk_index = 0;

    while (1) {
        // Acquire a mutex lock to ensure thread safety when accessing shared data
        pthread_mutex_lock(&mutex);
        if (current_chunk >= thread_data->num_chunks) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        int chunk_index = current_chunk++;
        pthread_mutex_unlock(&mutex);

        // Get the chunk of data to compress
        chunk_t chunk = thread_data->chunks[chunk_index];
        size_t compressed_size = 0;
        char *compressed_data = malloc(chunk.size * sizeof(char));

        char prev_char = chunk.data[0];
        int count = 1;

        // Compress the chunk using RLE
        for (size_t i = 1; i <= chunk.size; i++) {
            if (i < chunk.size && chunk.data[i] == prev_char) {
                count++;
            } else {
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&prev_char, sizeof(char), 1, stdout);
                compressed_size += sizeof(int) + sizeof(char);

                if (i < chunk.size) {
                    prev_char = chunk.data[i];
                    count = 1;
                }
            }
        }

        // Store the compressed chunk in the thread-local buffer
        compressed_chunks[compressed_chunk_index].data = compressed_data;
        compressed_chunks[compressed_chunk_index].size = compressed_size;
        compressed_chunk_index++;
    }

    // Write the compressed chunks to the output file in the correct order
    for (int i = 0; i < compressed_chunk_index; i++) {
        fwrite(compressed_chunks[i].data, sizeof(char), compressed_chunks[i].size, stdout);
        free(compressed_chunks[i].data);
    }

    free(compressed_chunks);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    }

    // Determine the number of threads to create based on the number of CPU resources
    int num_threads = get_nprocs();
    if (num_threads > MAX_THREADS) {
        num_threads = MAX_THREADS;
    }

    // Initialize the thread data structure
    thread_data_t thread_data;
    thread_data.num_chunks = argc - 1;
    thread_data.chunks = malloc(thread_data.num_chunks * sizeof(chunk_t));

    // Read the contents of each input file into memory
    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file: %s\n", argv[i]);
            exit(1);
        }

        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *data = malloc(file_size * sizeof(char));
        fread(data, sizeof(char), file_size, file);
        fclose(file);

        thread_data.chunks[i - 1].data = data;
        thread_data.chunks[i - 1].size = file_size;
    }

    // Create the threads and pass the thread data to each thread
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, compress_chunk, &thread_data);
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

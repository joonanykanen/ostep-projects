// reverse.c, JN, 05.05.2024

// Include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Define a struct to represent a node in a doubly linked list
typedef struct Node {
    char *line;           // Stores a line of text
    struct Node *next;    // Points to the next node in the list
    struct Node *prev;    // Points to the previous node in the list
} Node;

// Inserts a new node at the tail of the linked list
void insertAtTail(Node **head, Node **tail, char *line) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    newNode->line = strdup(line);
    newNode->next = NULL;
    newNode->prev = *tail;

    // Update the next pointer of the previous tail node
    if (*tail != NULL)
        (*tail)->next = newNode;
    else
        *head = newNode;

    *tail = newNode;
}

// Prints the linked list in reverse order to the output file
void printReverse(Node *tail, FILE *output) {
    Node *current = tail;
    while (current != NULL) {
        fprintf(output, "%s", current->line);
        current = current->prev;
    }
}

// Frees the memory allocated for the linked list
void freeList(Node *head) {
    Node *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->line);
        free(temp);
    }
}

int main(int argc, char *argv[]) {
    // Default input and output files
    FILE *input = stdin;
    FILE *output = stdout;

    // Check command-line arguments
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    // Open input file if specified
    if (argc >= 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    // Check if input and output files are the same
    if (argc == 3) {
        struct stat inputStat, outputStat;
        if (stat(argv[1], &inputStat) == 0 && stat(argv[2], &outputStat) == 0) {
            if (inputStat.st_dev == outputStat.st_dev && inputStat.st_ino == outputStat.st_ino) {
                fprintf(stderr, "reverse: input and output file must differ\n");
                exit(1);
            }
        }
        output = fopen(argv[2], "w");
        if (output == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
    }

    // Initialize linked list
    Node *head = NULL;
    Node *tail = NULL;
    char *line = NULL;
    size_t len = 0;
    size_t read;

    // Read lines from input file and insert into linked list
    while ((read = getline(&line, &len, input)) != -1) {
        insertAtTail(&head, &tail, line);
    }

    free(line);

    // Print linked list in reverse order to output file
    printReverse(tail, output);

    // Free memory allocated for linked list
    freeList(head);

    // Close files if opened
    if (argc >= 2)
        fclose(input);
    if (argc == 3)
        fclose(output);

    return 0;
}

// eof

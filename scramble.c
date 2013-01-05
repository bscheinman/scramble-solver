#include <stdio.h>
#include <string.h>

#include "trie.h"

#define BOARD_HEIGHT 4
#define BOARD_WIDTH 4
#define LINE_LENGTH 80

char board[BOARD_WIDTH][BOARD_HEIGHT];
trie *dict_trie;


typedef struct {
    int x, y;
    char *prefix;
    trie *words;
    int visited; /* this serves as a bitmask of which characters are contained in the current path */
} path;


#define as_bitmask(i, j) 1 << (15 - (3 * (i)) - (j))

/* coordinate changes for each possible move */
#define MOVE_COUNT 8
int dx[MOVE_COUNT] = { -1, 0, 1, -1, 1, -1, 0, 1 };
int dy[MOVE_COUNT] = { -1, -1, -1, 0, 0, 1, 1, 1 };

void print_words_impl(int i, int j)
{
    path start = { i, j, "", dict_trie, as_bitmask(i, j)};
    linked_list *visits;
    list_initialize(visits);
    queue_push(visits, &start);

    while (!list_empty(visits)) {
        path *node = (path *)queue_pop(visits);
        size_t prefix_length = strlen(node->prefix);
        char *word = malloc(sizeof(char) * (prefix_length + 2));
        strcpy(word, node->prefix);
        word[prefix_length + 1] = '\0';

        /* check all adjacent spaces */
        for (int m = 0 ; m < MOVE_COUNT ; ++m) {
            i = node->x + dx[m];
            j = node->y + dy[m];

            /* ignore spaces that are off the board */
            if (i < 0 || i >= BOARD_WIDTH || j < 0 || j >= BOARD_HEIGHT) continue;
            
            /* we can't visit the same space twice in one word */
            if (node->visited & as_bitmask(i, j)) continue;

            char letter = board[i][j];
            trie *children = trie_get_child(node->words, letter);

            /* if there aren't any possible words on this path then we can stop here */
            if (!children) continue;

            word[prefix_length] = letter;
            /* if this is a valid word itself, then print it */
            if (children->is_word)
                printf("%s\n", word);

            /* create next node to visit and put it on the queue */
            path *next = malloc(sizeof(path));
            next->x = i;
            next->y = j;
            next->prefix = malloc(sizeof(char) * (strlen(word) + 1));
            strcpy(next->prefix, word);
            next->words = children;
            next->visited = node->visited | as_bitmask(i,j);
            queue_push(visits, next);
        }

        /* be sure to free temporary data when we're done processing it */
        free(word);
        free(node->prefix);
        free(node);
    }
}


void print_words(void)
{
    for (int i = 0 ; i < BOARD_WIDTH ; ++i)
        for (int j = 0 ; j < BOARD_HEIGHT ; ++j)
            print_words_impl(i, j);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: scramble dict_file board_file\n");
        return 1;
    }

    FILE *board_file = fopen(argv[2], 'r');
    char line[LINE_LENGTH];
    int line_no = 0;

    while (line_no < BOARD_HEIGHT && fgets(line, BOARD_WIDTH + 2, board_file)) {
        for (size_t i = 0 ; i < BOARD_WIDTH ; ++i) {
            if (!isalpha(line[i])) {
                printf("all spaces must contain letters\n");
                return 2;
            }
            line[i] = tolower(line[i]);
        }
        strncpy(board[line_no++], line, BOARD_WIDTH);
    }
    fclose(board_file);


    FILE *dict_file = fopen(argv[1], 'r');
    dict_trie = trie_init();

    while (fgets(line, LINE_LENGTH, dict_file)) {
        trie_insert(dict_trie, line);
    }
    fclose(dict_file);
    
    print_words();

    trie_dispose(dict_trie);
}

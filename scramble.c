#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "trie.h"

#define BOARD_HEIGHT 4
#define BOARD_WIDTH 4
#define LINE_LENGTH 80

#define RESULT_SUCCESS 0
#define RESULT_INVALID_USAGE 1
#define RESULT_INVALID_BOARD 2

char board[BOARD_WIDTH][BOARD_HEIGHT];
trie *dict_trie;


typedef struct {
    int x, y;
    char *prefix;
    int score;
    trie *words;
    int visited; /* this serves as a bitmask of which characters are contained in the current path */
} path;


/* this won't work with much bigger boards */
#define as_bitmask(x, y) 1 << (BOARD_HEIGHT * (i) + (j))

/* coordinate changes for each possible move */
#define MOVE_COUNT 8
int dx[MOVE_COUNT] = { -1, 0, 1, -1, 1, -1, 0, 1 };
int dy[MOVE_COUNT] = { -1, -1, -1, 0, 0, 1, 1, 1 };

int letter_scores[ALPHABET_SIZE] = {
    /* a */ 1,
    /* b */ 4,
    /* c */ 4,
    /* d */ 2,
    /* e */ 1,
    /* f */ 4,
    /* g */ 3,
    /* h */ 3,
    /* i */ 1,
    /* j */ 10,
    /* k */ 5,
    /* l */ 2,
    /* m */ 4,
    /* n */ 2,
    /* o */ 1,
    /* p */ 4,
    /* q */ 10,
    /* r */ 1,
    /* s */ 1,
    /* t */ 1,
    /* u */ 2,
    /* v */ 5,
    /* w */ 4,
    /* x */ 8,
    /* y */ 3,
    /* z */ 10
};

void print_words_impl(int i, int j)
{
    char letter = board[i][j];
    path *start = malloc(sizeof(path));
    start->x = i;
    start->y = j;
    start->score = letter_scores[letter - 'a'];

    start->prefix = malloc(sizeof(char) * 2);
    *start->prefix = letter;
    *(start->prefix + 1) = '\0';

    start->words = trie_get_child(dict_trie, letter);
    /* if there are no words that start with this letter (doesn't occur in practice) then
       we don't need to go any further */
    if (!start->words) {
        free(start->prefix);
        free(start);
        return;
    }
    start->visited = as_bitmask(i, j);

    /* initialize list of nodes to visit */
    linked_list *visits = malloc(sizeof(linked_list));
    list_init(visits);
    queue_push(visits, start);

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

            letter = board[i][j];
            trie *children = trie_get_child(node->words, letter);

            /* if there aren't any possible words on this path then we can stop here */
            if (!children) continue;

            word[prefix_length] = letter;
            int word_score = node->score + letter_scores[letter - 'a'];
            /* if this is a valid word itself, then print it */
            if (children->is_word)
                printf("%s %i\n", word, word_score);

            /* create next node to visit and put it on the queue */
            path *next = malloc(sizeof(path));
            next->x = i;
            next->y = j;
            next->score = word_score;
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

    /* starting node has already been freed in the above loop */
    free(visits);
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
        return RESULT_INVALID_USAGE;
    }

    FILE *board_file = fopen(argv[2], "r");
    char line[LINE_LENGTH];
    int line_no = 0;

    while (line_no < BOARD_HEIGHT && fgets(line, BOARD_WIDTH + 2, board_file)) {
        for (size_t i = 0 ; i < BOARD_WIDTH ; ++i) {
            if (!isalpha(line[i])) {
                printf("all spaces must contain letters\n");
                return RESULT_INVALID_BOARD;
            }
            line[i] = tolower(line[i]);
        }
        strncpy(board[line_no++], line, BOARD_WIDTH);
    }
    if (line_no < BOARD_HEIGHT) {
        printf("you must provide %i lines\n", BOARD_HEIGHT);
        return RESULT_INVALID_BOARD;
    }
    fclose(board_file);


    FILE *dict_file = fopen(argv[1], "r");
    dict_trie = trie_init();

    while (fgets(line, LINE_LENGTH, dict_file)) {
        /* remove trailing newlines before inserting into trie */
        char *pos;
        if ((pos = strchr(line, '\n'))) *pos = '\0';
        trie_insert(dict_trie, line);
    }
    fclose(dict_file);
    
    print_words();

    trie_dispose(dict_trie);

    return RESULT_SUCCESS;
}

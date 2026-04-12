#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SIZE 26
#define MAX 50
#define MAX_SUG 200

// Trie Node
typedef struct Trie {
    struct Trie* child[SIZE];
    int end;
} Trie;

// Suggestion struct
typedef struct {
    char word[MAX];
    int dist;
} Suggestion;

Suggestion suggestions[MAX_SUG];
int sugCount = 0;

// Create node
Trie* createNode() {
    Trie* node = (Trie*)malloc(sizeof(Trie));
    node->end = 0;
    for (int i = 0; i < SIZE; i++)
        node->child[i] = NULL;
    return node;
}

// Insert word
void insert(Trie* root, char* word) {
    Trie* temp = root;
    for (int i = 0; word[i]; i++) {
        int idx = tolower(word[i]) - 'a';
        if (idx < 0 || idx >= 26) continue;

        if (!temp->child[idx])
            temp->child[idx] = createNode();

        temp = temp->child[idx];
    }
    temp->end = 1;
}

// Search word
int search(Trie* root, char* word) {
    Trie* temp = root;
    for (int i = 0; word[i]; i++) {
        int idx = tolower(word[i]) - 'a';
        if (idx < 0 || idx >= 26) continue;

        if (!temp->child[idx])
            return 0;

        temp = temp->child[idx];
    }
    return temp->end;
}

// Min function
int min(int a, int b, int c) {
    int m = (a < b) ? a : b;
    return (m < c) ? m : c;
}

// Edit Distance
int editDistance(char *a, char *b) {
    int m = strlen(a), n = strlen(b);
    int dp[m+1][n+1];

    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0) dp[i][j] = j;
            else if (j == 0) dp[i][j] = i;
            else if (tolower(a[i-1]) == tolower(b[j-1]))
                dp[i][j] = dp[i-1][j-1];
            else
                dp[i][j] = 1 + min(dp[i-1][j],
                                   dp[i][j-1],
                                   dp[i-1][j-1]);
        }
    }
    return dp[m][n];
}

// 🔥 Soundex (Phonetic Matching)
void soundex(char *word, char *code) {
    char map[26] = {
        '0','1','2','3','0','1','2','0','0','2','2','4','5',
        '5','0','1','2','6','2','3','0','1','0','2','0','2'
    };

    code[0] = toupper(word[0]);
    int j = 1;

    for (int i = 1; word[i] && j < 4; i++) {
        char c = map[tolower(word[i]) - 'a'];
        if (c != '0') {
            code[j++] = c;
        }
    }

    while (j < 4) code[j++] = '0';
    code[4] = '\0';
}

// Collect suggestions
void collect(Trie* root, char* buffer, int level, char* input) {
    if (root->end) {
        buffer[level] = '\0';

        int dist = editDistance(input, buffer);

        char code1[5], code2[5];
        soundex(input, code1);
        soundex(buffer, code2);

        // Condition: either edit distance OR phonetic match
        if ((dist <= 2 || strcmp(code1, code2) == 0) && sugCount < MAX_SUG) {
            strcpy(suggestions[sugCount].word, buffer);
            suggestions[sugCount].dist = dist;
            sugCount++;
        }
    }

    for (int i = 0; i < SIZE; i++) {
        if (root->child[i]) {
            buffer[level] = i + 'a';
            collect(root->child[i], buffer, level + 1, input);
        }
    }
}

// Sort suggestions
void sortSuggestions() {
    for (int i = 0; i < sugCount - 1; i++) {
        for (int j = i + 1; j < sugCount; j++) {
            if (suggestions[i].dist > suggestions[j].dist) {
                Suggestion temp = suggestions[i];
                suggestions[i] = suggestions[j];
                suggestions[j] = temp;
            }
        }
    }
}

// Load dictionary
void loadDictionary(Trie* root) {
    FILE *fp = fopen("dictionary.txt", "r");
    char word[MAX];

    if (!fp) {
        printf("dictionary.txt not found!\n");
        return;
    }

    while (fscanf(fp, "%s", word) != EOF) {
        insert(root, word);
    }

    fclose(fp);
}

// Save word
void saveWord(char* word) {
    FILE *fp = fopen("dictionary.txt", "a");
    fprintf(fp, "%s\n", word);
    fclose(fp);
}

// Main
int main() {
    Trie* root = createNode();
    loadDictionary(root);

    int choice;
    char word[MAX];
    while (1) {
        printf("\n1. Check Word\n2. Add Word\n3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Enter word: ");
            scanf("%s", word);

            if (search(root, word)) {
                printf("Correct word!\n");
            } else {
                printf("Incorrect word. Suggestions:\n");

                char buffer[MAX];
                sugCount = 0;

                collect(root, buffer, 0, word);
                sortSuggestions();

                int limit = (sugCount < 5) ? sugCount : 5;

                for (int i = 0; i < limit; i++) {
                    printf("%s (dist=%d)\n",
                           suggestions[i].word,
                           suggestions[i].dist);
                }

                if (limit == 0)
                    printf("No suggestions found\n");
            }
        }

        else if (choice == 2) {
            printf("Enter new word: ");
            scanf("%s", word);

            insert(root, word);
            saveWord(word);

            printf("Word added successfully!\n");
        }

        else if (choice == 3) {
            break;
        }

        else {
            printf("Invalid choice!\n");
        }
    }

    return 0;
}
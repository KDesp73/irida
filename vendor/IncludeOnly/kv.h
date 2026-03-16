// TODO: add to IncludeOnly/cli.h

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    char key[32];
    char value[1024];
} ConfigPair;

#ifndef MAX_CONFIG_PAIRS
    #define MAX_CONFIG_PAIRS 16
#endif
static ConfigPair kv_pairs[MAX_CONFIG_PAIRS];
static int kv_count = 0;

static const char* kv_get(const char* key, const char* default_value)
{
    for (int i = 0; i < kv_count; i++) {
        if (strcmp(kv_pairs[i].key, key) == 0) {
            return kv_pairs[i].value;
        }
    }
    return default_value;
}


static void kv_parse(int argc, char** argv)
{
    for (int i = 1; i < argc && kv_count < MAX_CONFIG_PAIRS; i++) {
        char* arg = argv[i];
        char* eq = strchr(arg, '=');
        
        if (eq) {
            // 1. Extract Key
            size_t keyLen = eq - arg;
            if (keyLen >= 32) keyLen = 31;
            strncpy(kv_pairs[kv_count].key, arg, keyLen);
            kv_pairs[kv_count].key[keyLen] = '\0';

            char* valStart = eq + 1;
            char tempValue[1024] = {0};

            // 2. Handle Quoted Values
            if (*valStart == '"' || *valStart == '\'') {
                char quote = *valStart;
                valStart++; // Skip opening quote
                
                // Copy the part of the value in the current argv
                strcat(tempValue, valStart);

                // If the closing quote isn't in this argv, look ahead
                while (tempValue[strlen(tempValue)-1] != quote && (i + 1) < argc) {
                    i++;
                    strcat(tempValue, " "); // Add the space back
                    strcat(tempValue, argv[i]);
                }

                // Remove the trailing quote
                if (tempValue[strlen(tempValue)-1] == quote) {
                    tempValue[strlen(tempValue)-1] = '\0';
                }
            } else {
                // Standard unquoted value
                strncpy(tempValue, valStart, 1023);
            }

            strncpy(kv_pairs[kv_count].value, tempValue, 1023);
            kv_count++;
        }
    }
}

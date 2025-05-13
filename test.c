#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <string.h>

void init_ui() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
}

int create_directory(const char* path) {
    #ifdef _WIN32
        return _mkdir(path);
    #else
        return mkdir(path, 0777);
    #endif
}

void create_structure_from_json(cJSON *node, const char *base_path) {
    if (node == NULL) return;

    // Handle object (directory with contents)
    if (cJSON_IsObject(node)) {
        cJSON *child = node->child;
        while (child != NULL) {
            char new_path[1024];
            snprintf(new_path, sizeof(new_path), "%s/%s", base_path, child->string);
            
            // Create the directory
            if (create_directory(new_path) != 0) {
                mvprintw(0, 0, "Failed to create directory: %s", new_path);
                refresh();
            } else {
                mvprintw(0, 0, "Created directory: %s", new_path);
                clrtoeol();  // Clear to end of line
                refresh();
            }
            
            // Recursively process contents
            create_structure_from_json(child, new_path);
            child = child->next;
        }
    }
    // Handle array (directory that might contain nested structures)
    else if (cJSON_IsArray(node)) {
        int array_size = cJSON_GetArraySize(node);
        
        // Special case: array contains objects (like your Pics array)
        if (array_size > 0 && cJSON_IsObject(cJSON_GetArrayItem(node, 0))) {
            for (int i = 0; i < array_size; i++) {
                cJSON *array_item = cJSON_GetArrayItem(node, i);
                create_structure_from_json(array_item, base_path);
            }
        }
        // Empty array case (like your Logs array)
        else {
            // Directory already created, nothing more to do
        }
    }
}

cJSON* read_and_parse_json(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        mvprintw(0, 0, "Error: Could not open %s", filename);
        refresh();
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        fclose(file);
        mvprintw(0, 0, "Error: Memory allocation failed");
        refresh();
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            mvprintw(0, 0, "JSON Error before: %s", error_ptr);
            refresh();
        }
        return NULL;
    }

    return json;
}

int main() {
    init_ui();
    
    const char* filename = "handalf.json";
    cJSON *json = read_and_parse_json(filename);
    
    if (json != NULL) {
        create_structure_from_json(json, ".");
        cJSON_Delete(json);
    }

    mvprintw(1, 0, "Directory structure created. Press any key to exit...");
    refresh();
    getch();
    endwin();
    
    return 0;
}
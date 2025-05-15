#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <string.h>



void init_ui(){
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
		#include <direct.h>
		return _mkdir(path);
	#else
		#include <unistd.h>
		return mkdir(path, 0777);
	#endif
}

void create_file(const char *path) {
	FILE *file = fopen(path, "w");
	if (file == NULL) {
		mvprintw(0, 0, "Error: Could not create file %s", path);
		refresh();
	}
	else {
		refresh();
		fclose(file);
	}
}

void write_csv(const char *path, const char *headers) {
	printf("check %s %s", path, headers);
}

void create_structure(cJSON *node, const char *base_path) {
    if (node == NULL) {
        mvprintw(0, 0, "Error: JSON node is NULL");
        refresh();
        return;
    }

    if (cJSON_IsObject(node)) {
        cJSON *child = node->child;
		const char *file_name = NULL;
        while (child != NULL) {
            if (strcmp(child->string, "class") == 0) {
                if (child->next != NULL) {
					file_name = child->next->string;
				}
				child = child->next;
                continue;
            }

			cJSON *type_item = cJSON_GetObjectItem(child, "type");
			const char *node_type = (type_item && cJSON_IsString(type_item)) ? type_item->valuestring : NULL;
			if (node_type && strcmp(node_type, "csv") == 0) {
				cJSON *headers = cJSON_GetObjectItem(child, "headers");
        		const char *headers_str = (headers && cJSON_IsString(headers)) ? headers->valuestring : NULL;
				char file_path[1024];
				snprintf(file_path, sizeof(file_path), "%s/%s", base_path, file_name);
        		write_csv(file_path, headers_str);
			}

            cJSON *class_item = cJSON_GetObjectItem(child, "class");
            const char *node_class = (class_item && cJSON_IsString(class_item)) ? class_item->valuestring : NULL;

            char new_path[1024];
            snprintf(new_path, sizeof(new_path), "%s/%s", base_path, child->string);

            if (node_class) {
                if (strcmp(node_class, "file") == 0) {
                    create_file(new_path);
                    mvprintw(0, 2, "Created file: %s", new_path);
                    refresh();
                } else if (strcmp(node_class, "folder") == 0) {
                    if (create_directory(new_path) != 0) {
                        mvprintw(0, 1, "Failed to create directory: %s", new_path);
                        refresh();
                    } else {
                        mvprintw(0, 2, "Created directory: %s", new_path);
                        refresh();
                    }
                    create_structure(child, new_path);
                }
            }
            child = child->next;
        }
    } else if (cJSON_IsArray(node)) {
        int array_size = cJSON_GetArraySize(node);
        for (int i = 0; i < array_size; i++) {
            cJSON *array_item = cJSON_GetArrayItem(node, i);
            create_structure(array_item, base_path);
        }
    }
}

char* read_file() {
	const char* filename = "handalf.json";
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		mvprintw(0, 0, "Error: Could not open handalf.json in current directory");
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
	
	char *file_content = buffer;
	if (file_content == NULL) {
		mvprintw(0, 0, "Error: Failed to read file content");
		refresh();
		return NULL;
	}

	cJSON *json = cJSON_Parse(file_content);
	free(file_content);

	if (json == NULL) {
		mvprintw(0, 0, "Error: Failed to parse handalf.json");
		refresh();
		return NULL;
	}

	create_structure(json, ".");

	char* json_string = cJSON_Print(json);
	cJSON_Delete(json);
	return json_string;
}

int main() {

	init_ui();
	mvprintw(0, 0, read_file());
	refresh();
	getch();
	endwin();
	return 0;
}

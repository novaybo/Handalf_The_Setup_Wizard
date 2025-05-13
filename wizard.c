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

void create_structure(cJSON *node, const char *base_path) {
	if (node == NULL) {
		mvprintw(0, 0, "Error: JSON node is NULL");
		refresh();
		return;
	}

	if (cJSON_IsObject(node)) {
		cJSON *child = node->child;
		while (child != NULL) {
			char new_path[1024];
			snprintf(new_path, sizeof(new_path), "%s/%s", base_path, child->string);
			
			if (create_directory(new_path) != 0) {
				mvprintw(0, 0, "Failed to create directory: %s", new_path);
				refresh();
			} 
			else {
				mvprintw(0, 0, "Created directory: %s", new_path);
				refresh();
			}

			create_structure(child, new_path);
			child = child->next;
		}
	}
	else if (cJSON_IsArray(node)) {
		int array_size = cJSON_GetArraySize(node);

		if (array_size > 0 && cJSON_IsObject(cJSON_GetArrayItem(node, 0))) {
			for (int i = 0; i < array_size; i++) {
				cJSON *array_item = cJSON_GetArrayItem(node, i);
				create_structure(array_item, base_path);
			}
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
	free(read_file());
	return 0;
}

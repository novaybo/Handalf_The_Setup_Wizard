#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <string.h>
#include <ncurses.h>
#include <errno.h>
#include <unistd.h>

#define STATUS_HEIGHT 10
#define STATUS_WIDTH 70
#define INFO_HEIGHT 5
#define INFO_WIDTH 70
#define MAX_STATUS_LINES (STATUS_HEIGHT -2)
#define MAX_STATUS_MSG_LEN 256

char status_msgs[MAX_STATUS_LINES][MAX_STATUS_MSG_LEN];
int status_msg_count = 0;
int status_msg_start = 0;

WINDOW *status_win, *info_win;

#define PROGRESS_HEIGHT 3
#define PROGRESS_WIDTH 70

int total_items = 0;
int items_done = 0;

WINDOW *progress_win;



void init_ui(){
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	refresh();
}

void welcome_page() {
	clear();
	attron(A_BOLD | COLOR_PAIR(1));
	mvprintw(2, 10, "Handalf The Setup Wizard");
	attroff(A_BOLD | COLOR_PAIR(1));
	mvprintw(4, 10, "This wizard will set up your project structue as defined in handalf.json");
	mvprintw(6, 10, "Press any key to start -> -> ->");
	refresh();
	int input;

	while (1) {
		input = getch();
		if (input == 27) {
			endwin();
			exit(0);
		} else if (input == '\n' || input == KEY_ENTER || input == 10 || input == 13) {
			break;
		}
	}
	clear();
	refresh();
}

void create_windows() {
	int starty = 2, startx = 5;
	status_win = newwin(STATUS_HEIGHT, STATUS_WIDTH, starty, startx);
	mvwprintw(status_win, 0, 2, " Progress ");
	wrefresh(status_win);

	info_win = newwin(INFO_HEIGHT, INFO_WIDTH, starty + STATUS_HEIGHT + 1, startx);
	box(info_win, 0, 0);
	mvwprintw(info_win, 0, 2, " Info ");
	wrefresh(info_win);
}

void update_status(const char *msg, int color_pair) {
	int idx = (status_msg_start + status_msg_count) % MAX_STATUS_LINES;
    snprintf(status_msgs[idx], MAX_STATUS_MSG_LEN, "%s", msg);

    if (status_msg_count < MAX_STATUS_LINES) {
        status_msg_count++;
    } else {
        status_msg_start = (status_msg_start + 1) % MAX_STATUS_LINES;
    }

    werase(status_win);
	wattron(status_win, COLOR_PAIR(2));
    box(status_win, 0, 0);
    mvwprintw(status_win, 0, 2, " Progress ");
	wattroff(status_win, COLOR_PAIR(2));

    for (int i = 0; i < status_msg_count; ++i) {
        int msg_idx = (status_msg_start + i) % MAX_STATUS_LINES;
        wattron(status_win, COLOR_PAIR(color_pair));
        mvwprintw(status_win, i + 1, 2, "%s", status_msgs[msg_idx]);
        wattroff(status_win, COLOR_PAIR(color_pair));
    }
    wrefresh(status_win);
}

void show_info(const char *msg, int color_pair) {
	werase(info_win);
	wattron(info_win, COLOR_PAIR(2));
	box(info_win, 0, 0);
	mvwprintw(info_win, 0, 2, " Info ");
	wattroff(info_win, COLOR_PAIR(2));
	wattron(info_win, COLOR_PAIR(color_pair));
	mvwprintw(info_win, 2, 2, "%s", msg);
	wattroff(info_win, COLOR_PAIR(color_pair));
	wrefresh(info_win);
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
		char buf[256];
		snprintf(buf, sizeof(buf), "Error: Could not create file %.200s", path);
		update_status(buf, 3);
	}
	else {
		fclose(file);
	}
}

void write_csv(const char *path, const char *headers) {
	FILE *file = fopen(path, "w");
	if (file != NULL) {
		fprintf(file, "%s\n", headers);
		fclose(file);
	}
	else {
		char buf[256];
		snprintf(buf, sizeof(buf), "Failed to write headers %s", path);
		update_status(buf, 3);
	}
}
void update_progress_bar(int done, int total) {
	float percent = (total > 0) ? ((float)done / total) : 0;
	int bar_width = PROGRESS_WIDTH - 4;
	int filled = (int)(percent * (bar_width));

	werase(progress_win);
	wattron(progress_win, COLOR_PAIR(2));
	box(progress_win, 0, 0);
	mvwprintw(progress_win, 0, 2, " Progress Bar ");
	wattroff(progress_win, COLOR_PAIR(2));

	wattron(progress_win, COLOR_PAIR(1));
	mvwprintw(progress_win, 1, 2, "[");
	for (int i = 0; i < bar_width - 1; ++i) {
		if (i < filled)
			waddch(progress_win, '=');
		else
			waddch(progress_win, ' ');
	}
	mvwaddch(progress_win, 1, 2 + bar_width - 1, ']');
	wattroff(progress_win, COLOR_PAIR(1));
	wattron(progress_win, COLOR_PAIR(2));
	mvwprintw(progress_win, 2, 2, " %d%% ", (int)(percent * 100));
	wattroff(progress_win, COLOR_PAIR(2));
	wrefresh(progress_win);
}

void create_structure(cJSON *node, const char *base_path) {
    if (node == NULL) {
		update_status("Error: JSON node is NULL", 3);
		return;
    }

    if (cJSON_IsObject(node)) {
        cJSON *child = node->child;
		char *file_name = NULL;
		char file_path[1024];
		char *headers_str = NULL;
        while (child != NULL) {
            if (strcmp(child->string, "class") == 0) {
				child = child->next;
                continue;
            }

			

            cJSON *class_item = cJSON_GetObjectItem(child, "class");
            const char *node_class = (class_item && cJSON_IsString(class_item)) ? class_item->valuestring : NULL;

            char new_path[1024];
            snprintf(new_path, sizeof(new_path), "%s/%s", base_path, child->string);

            if (node_class) {
                if (strcmp(node_class, "file") == 0) {
                    create_file(new_path);
					char buf[256];
					snprintf(buf, sizeof(buf), "Created file %.200s", new_path);
					update_status(buf, 1);
					items_done++;
					update_progress_bar(items_done, total_items);
                } else if (strcmp(node_class, "folder") == 0) {
                    if (create_directory(new_path) != 0 && create_directory(new_path) != -1) {
						char buf[256];
						snprintf(buf, sizeof(buf), "Failed to create directory: %.200s", new_path);
						update_status(buf, 3);
					}
					else {
						char buf[256];
						snprintf(buf, sizeof(buf), "Created directory %.200s", new_path);
						update_status(buf, 1);
						items_done++;
						update_progress_bar(items_done, total_items);
					}
                    create_structure(child, new_path);
                }
            }
			
			cJSON *type_item = cJSON_GetObjectItem(child, "type");
			const char *node_type = (type_item && cJSON_IsString(type_item)) ? type_item->valuestring : NULL;
			if (node_type && strcmp(node_type, "csv") == 0) {
				cJSON *headers = cJSON_GetObjectItem(child, "headers");
        		headers_str = (headers && cJSON_IsString(headers)) ? headers->valuestring : NULL;
				file_name = child->string;
				snprintf(file_path, sizeof(file_path), "%s/%s", base_path, file_name);
        		write_csv(file_path, headers_str);
				char buf[256];
				snprintf(buf, sizeof(buf), "Created CSV: %.200s", file_path);
				update_status(buf, 2);
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

int count_items(cJSON *node) {
	int count = 0;
	if (node == NULL) return 0;
	if (cJSON_IsObject(node)) {
		cJSON *child = node->child;
		while (child != NULL) {
			cJSON *class_item = cJSON_GetObjectItem(child, "class");
			const char *node_class = (class_item && cJSON_IsString(class_item)) ? class_item->valuestring : NULL;
			if (node_class && (strcmp(node_class, "file") == 0 || strcmp(node_class, "folder") == 0)) {
				count++;
			}
			count += count_items(child);
			child = child->next;
		}
	}else if (cJSON_IsArray(node)) {
		int array_size = cJSON_GetArraySize(node);
		for (int i = 0; i < array_size; i++) {
			cJSON *array_item = cJSON_GetArrayItem(node, i);
			count += count_items(array_item);
		}
	}
	return count;
}

int move_item(const char *src, const char *dst) {
	char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mv \"%s\" \"%s\"", src, dst);
    int result = system(cmd);

    if (result == 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Moved %s to %s (using system mv)", src, dst);
        update_status(buf, 1);
        return 0;
    } else {
        char buf[256];
        snprintf(buf, sizeof(buf), "Error: Could not move from %s to %s (using system mv, result=%d)", src, dst, result);
        update_status(buf, 3);
        return -1;
    }
}

void process_moves(cJSON *move_section) {
	if (move_section == NULL || !cJSON_IsObject(move_section)) {
		update_status("Error: Move section is NULL or not an object", 3);
		return;
	}

	cJSON *move_entry = move_section->child;
	while(move_entry) {
		cJSON *class_item = cJSON_GetObjectItem(move_entry, "class");
		const char *node_class = (class_item && cJSON_IsString(class_item)) ? class_item->valuestring : NULL;

		cJSON *from_item = cJSON_GetObjectItem(move_entry, "from");
		const char *from = (from_item && cJSON_IsString(from_item)) ? from_item->valuestring : NULL;

		cJSON *move_to_item = cJSON_GetObjectItem(move_entry, "move to");
		const char *move_to = (move_to_item && cJSON_IsString(move_to_item)) ? move_to_item->valuestring : NULL;

		if (node_class && move_to && from) {
			char src_path[1024];
			char dst_path[1024];
			snprintf(src_path, sizeof(src_path), "%s/%s", from, move_entry->string);
			snprintf(dst_path, sizeof(dst_path), "%s/%s", move_to, move_entry->string);

			move_item(src_path, dst_path);

			items_done++;
			update_progress_bar(items_done, total_items);
		}
		move_entry = move_entry->next;
	}
}

int count_moves(cJSON *move_section) {
	int count = 0;
	if (!move_section || !cJSON_IsObject(move_section)) {
		update_status("Error: Move section is NULL or not an object", 3);
		return 0;
	}
	cJSON *move_entry = move_section->child;
	while (move_entry) {
		count++;
		move_entry = move_entry->next;
	}
	return count;
}

char* read_file() {
	const char* filename = "handalf.json";
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		show_info("Error: Could not open handalf.json in current directory", 3);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = (char*)malloc(length + 1);
	if (buffer == NULL) {
		fclose(file);
		show_info("Error: Memory allocation failed", 3);
		return NULL;
	}

	fread(buffer, 1, length, file);
	buffer[length] = '\0';

	fclose(file);
	
	char *file_content = buffer;
	if (file_content == NULL) {
		show_info("Error: Failed to read file content", 3);
		return NULL;
	}

	cJSON *json = cJSON_Parse(file_content);
	free(file_content);

	if (json == NULL) {
		show_info("Error: Failed to parse handalf.json", 3);
		return NULL;
	}

	cJSON *to_create = cJSON_GetObjectItem(json, "to-create");
	cJSON *to_move = cJSON_GetObjectItem(json, "to-move");
	
	if (to_create) total_items += count_items(to_create);
	if (to_move) total_items += count_moves(to_move);
	items_done = 0;

	if (to_create) create_structure(to_create, ".");
	if (to_move) process_moves(to_move);

	char* json_string = cJSON_Print(json);
	cJSON_Delete(json);
	return json_string;
}

void init_progrss_bar() {
	int starty = 2;
	int startx = STATUS_WIDTH + 15;
	progress_win = newwin(PROGRESS_HEIGHT, PROGRESS_WIDTH, starty, startx);
	wattron(progress_win, COLOR_PAIR(2));
	box(progress_win, 0, 0);
	mvwprintw(progress_win, 0, 2, " Progress Bar ");
	wattroff(progress_win, COLOR_PAIR(2));
	wrefresh(progress_win);
}

int main() {

	init_ui();
	welcome_page();
	init_progrss_bar();
	create_windows();
	show_info("Setting up project structure...", 2);
	char *json_str = read_file();
	if (json_str) {
		show_info("Setup complete! Press any key to exit.", 1);
		free(json_str);
	} else {
		show_info("Setup failed. Press any key to exit.", 3);
	}
	getch();
	delwin(status_win);
	delwin(info_win);
	delwin(progress_win);
	endwin();
	return 0;
}

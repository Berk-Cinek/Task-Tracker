#include <iostream>
#include <nlohmann/json.hpp>
#include <ctime>
#include <magic_enum.hpp>


using namespace nlohmann::literals;
using json = nlohmann::json;

enum TaskStatus { TODO, IN_PROGRESS, DONE };

struct Task {
	int id;
	std::string description;
	TaskStatus status;
	time_t CreatedAt;
	time_t UpdatedAt;
};

std::string status_to_string(TaskStatus T) {
	switch (T) {
	case TaskStatus::TODO: return "Todo";
	case TaskStatus::IN_PROGRESS: return "In progress";
	case TaskStatus::DONE: return "Done";
	}
}

TaskStatus string_to_status(std::string T) {
	if (T == "todo")
		return TaskStatus::TODO;
	else if (T == "in progress")
		return TaskStatus::IN_PROGRESS;
	else if (T == "done")
		return TaskStatus::DONE;
}

void to_json(json& j, const Task& t) {
	j = json{
	{"id", t.id},
	{"title", t.description},
	{"status", status_to_string(t.status)},
	{"created at", t.CreatedAt },
	{"updated at", t.UpdatedAt}
	};
}

void from_json(const json& j, Task& t) {
	t.id = j.at("id").get<int>();
	t.description = j.at("title").get<std::string>();
	t.status = string_to_status(j.at("status").get<std::string>());
	t.UpdatedAt = j.at("updated at").get<time_t>();
}

void add(const std::string& title);
void update();
void del();

int main(int argc, char* argv[])
{
	std::vector<std::string> args(argv, argv + argc);

	if (argc < 2) {
		std::cerr << "Usage: " << args[0] << " <command> [arguments...]" << std::endl;
		return 1;
	}

	std::string command = args[1];

	if (command == "add") {
		if (argc != 3) {
			std::cerr << "Error: 'add' requires a task title in quotes." << std::endl;
			std::cerr << "Usage: " << args[0] << " add \"<task title>\"" << std::endl;
			return 1;
		}

		std::string task_title = args[2];
		add(task_title);
	}
	else {
		std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
		return 1;
	}


}

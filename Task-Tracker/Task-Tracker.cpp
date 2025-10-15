#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>


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

using TaskCollection = std::map<int, Task>;

std::vector<std::string> split_line(const std::string& line) {
	std::stringstream ss(line);
	std::string token;
	std::vector<std::string> args;

	while (ss >> token) {
		args.push_back(token);
	}
	return args;
}

std::string status_to_string(TaskStatus T) {
	switch (T) {
	case TaskStatus::TODO: return "Todo";
	case TaskStatus::IN_PROGRESS: return "In_progress";
	case TaskStatus::DONE: return "Done";

	default: return "todo";
	}
}

TaskStatus string_to_status(std::string T) {
	if (T == "todo")
		return TaskStatus::TODO;
	else if (T == "in_progress")
		return TaskStatus::IN_PROGRESS;
	else if (T == "done")
		return TaskStatus::DONE;

	return TaskStatus::TODO;
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
	t.CreatedAt = j.at("created at").get<time_t>();
	t.UpdatedAt = j.at("updated at").get<time_t>();
}

TaskCollection load_task() {
	const std::string FILENAME = "tasks.json";
	std::ifstream file(FILENAME);
	TaskCollection tasks;
	try {
		if (file.is_open()) {
			json j = json::parse(file);
			for (auto it = j.begin(); it != j.end(); ++it) {
				Task t;
				int id = std::stoi(it.key());
				it.value().get_to(t);
				tasks[id] = t;
			}
		}
	}
	catch (const json::parse_error& e) {
		std::cerr << "Warning: Could not parse tasks.json. File may be empty or corrupt." << std::endl;
	}
	return tasks;
}

void save_tasks(const TaskCollection& tasks) {
	const std::string FILENAME = "tasks.json";
	std::ofstream file(FILENAME);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open tasks.json for writing!" << std::endl;
		return;
	}

	json j_out;
	for (const auto& pair : tasks) {
		j_out[std::to_string(pair.first)] = pair.second;
	}
	file << j_out.dump(4) << std::endl;
}

void add(const std::string& title);
void update(int id, const std::string& value);
void del(int id);

int main(int argc, char* argv[])
{
	std::cout << "--- C++ Task Tracker Shell ---" << std::endl;
	std::vector<std::string> args(argv, argv + argc);
	std::string input_line;

	while (true) {
		std::cout << "\ntask> ";

		if (!std::getline(std::cin, input_line)) {
			break;
		}
		std::vector<std::string> args = split_line(input_line);

		if (args.empty()) {
			continue;
		}

		std::string command = args[0];
		int argc = args.size();

		if (command == "exit") {
			std::cout << "Exiting Task Tracker. Goodbye!" << std::endl;
			break; 
		}
		else if (command == "add") {
			if (argc != 2) {
				std::cerr << "Error: 'add' requires a task title." << std::endl;
			}
			else {
				add(args[1]);
			}
		}
		else if (command == "update") {
			if (argc != 3) {
				std::cerr << "Error: 'update' requires <ID> and <value>." << std::endl;
			}
			else {
				try {
					int id = std::stoi(args[1]);
					std::string value = args[2];
					update(id, value);
				}
				catch (const std::invalid_argument& e) {
					std::cerr << "Error: Task ID must be a valid number." << std::endl;
				}
			}
		}
		else if (command == "del") {
			if (argc != 2) {
				std::cerr << "Error: 'del' requires a task ID." << std::endl;
			}
			else {
				try {
					int id = std::stoi(args[1]);
					del(id);
				}
				catch (const std::invalid_argument& e) {
					std::cerr << "Error: Task ID must be a valid number." << std::endl;
				}
			}
		}
		else {
			std::cerr << "Error: Unknown command '" << command << "'. Use 'add', 'update', 'del', or 'exit'." << std::endl;
		}
	}

	return 0;
	
}

void add(const std::string& description) {
	TaskCollection tasks = load_task();

	int new_id = 1;
	if (!tasks.empty()) {
		new_id = tasks.rbegin()->first + 1;
	}

	Task new_task;
	new_task.id = new_id;
	new_task.description = description;
	new_task.status = TaskStatus::TODO;
	new_task.CreatedAt = std::time(nullptr);
	new_task.UpdatedAt = std::time(nullptr);

	tasks[new_id] = new_task;
	save_tasks(tasks);

	std::cout << "Successfully added task #" << new_id << ":\"" << description << "\" (status : Todo)" << std::endl;


}

void update(int id, const std::string& value) {
	TaskCollection tasks = load_task();
	
	if (tasks.find(id) == tasks.end()) {
		std::cerr << "Error: Task ID #" << id << "cannot be found" << std::endl;
		return;
	}
	Task& task_to_update = tasks.at(id);
	bool updated = false;
	std::string lower_value = value;
	std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);

	if (lower_value == "todo" || lower_value == "in_progress" || lower_value == "done") {
		task_to_update.status = string_to_status(lower_value);
		std::cout << "Task #" << id << "status updated to " << status_to_string(task_to_update.status) << std::endl;
		updated = true;
	}else if(value.empty()) {
		std::cerr << "Error: New title or status cannot be empty." << std::endl;
		return;
	}else {
		task_to_update.description = value;
		std::cout << "Task #" << id << " title updated to: " << task_to_update.description << std::endl;
		updated = true;
	}

	if (updated) {
		task_to_update.UpdatedAt = std::time(nullptr);
		save_tasks(tasks);
	}

}

void del(int id) {
	TaskCollection tasks = load_task();

	if (tasks.find(id) == tasks.end()){
		std::cerr << "Error: Task ID #" << id << "cannot be found" << std::endl;
		return;
	}
	tasks.erase(id);
	std::cout << "task #" << id << "has been deleted" << std::endl;
	save_tasks(tasks);
}

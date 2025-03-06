#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

struct Config {
    bool help = false;
    std::string repl = "%";
    bool concat = false;
};

int replace_all(
    std::string& str,
    const std::string& from,
    const std::string& to) {
    if (from.empty()) {
        return 0;
    }
    int count = 0;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        count += 1;
    }
    return count;
}

void print_help(std::string basename) {
    std::vector<std::string> message = {
        "{}, version 0.2",
        "Line oriented version of xargs",
        "",
        "usage: {} [-ch] [-j replstr] [utility [argument ...]]",
    };
    for (auto line : message) {
        replace_all(line, "{}", basename);
        std::cout << line << std::endl;
    }
}

Config parse_args(int argc, char** argv, size_t& i) {
    Config config;
    std::string current_option = "";
    for (i = 1; i < argc; i++) {
        current_option = argv[i];
        if (not current_option.starts_with('-') ||
            current_option.length() != 2) {
            return config;
        }
        switch (current_option[1]) {
            case 'c': {
                config.concat = true;
                break;
            }
            case 'h': {
                config.help = true;
                break;
            }
            case 'j': {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Expects value for -j\n");
                    exit(1);
                }
                config.repl = argv[++i];
                break;
            }
        }
    }
    return config;
}

void spawn(char* cmd, std::vector<std::string> argv) {
    switch (fork()) {
        case -1: {
            perror("fork");
            exit(1);
        }
        case 0: {
            char** ptr_argv = new char*[argv.size() + 1];
            for (int i = 0; i < argv.size(); i++) {
                ptr_argv[i] = strdup(argv[i].c_str());
            }
            ptr_argv[argv.size()] = NULL;
            execvp(cmd, ptr_argv);
            perror("execvp");
            exit(1);
        }
        default: {
            return;
        }
    }
}

void separate_lines(
    int argc,
    char** argv,
    Config&& config,
    std::vector<std::string>&& inputs) {
    for (auto input : inputs) {
        char* cmd = strdup(argv[0]);

        std::vector<std::string> rargv = {cmd};

        int replace_count = 0;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == config.repl) {
                arg = input;
                replace_count += 1;
            }
            rargv.push_back(arg);
        }
        if (replace_count == 0) {
            rargv.push_back(input);
        }

        spawn(cmd, rargv);
    }
}

void same_lines(
    int argc,
    char** argv,
    Config&& config,
    std::vector<std::string>&& inputs) {
    char* cmd = strdup(argv[0]);

    int replace_count = 0;
    std::vector<std::string> rargv = {cmd};
    for (int i = 1; i < argc; i++) {
        if (argv[i] == config.repl) {
            replace_count += 1;
            rargv.insert(rargv.end(), inputs.begin(), inputs.end());
        } else {
            rargv.push_back(argv[i]);
        }
    }
    if (replace_count == 0) {
        rargv.insert(rargv.end(), inputs.begin(), inputs.end());
    }

    spawn(cmd, rargv);
}

int main(int argc, char** argv) {
    size_t argidx = 0;
    Config config = parse_args(argc, argv, argidx);

    if (config.help) {
        print_help(argv[0]);
        return 0;
    }

    std::vector<std::string> inputs;
    for (std::string input; std::getline(std::cin, input);
         inputs.push_back(input)) {
    }

    if (not config.concat) {
        separate_lines(
            argc - argidx, argv + argidx, std::move(config), std::move(inputs));
    } else {
        same_lines(
            argc - argidx, argv + argidx, std::move(config), std::move(inputs));
    }

    return 0;
}

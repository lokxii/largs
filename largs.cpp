#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

std::string shift(std::string arg, int count) {
    return std::string(arg.begin() + count, arg.end());
}

// hcj:
std::tuple<std::map<std::string, std::string>, std::vector<std::string>>
parse_args(int argc, char** argv) {
    std::map<std::string, std::string> map;
    std::optional<std::string> waiting_to_eat;
    int i = 1;
    for (; i < argc; i++) {
        std::string arg = argv[i];

        if (waiting_to_eat.has_value()) {
            map[waiting_to_eat.value()] = arg;

        } else if (arg[0] == '-') {
            arg = shift(arg, 1);
            while (arg.length() > 0) {
                switch (arg[0]) {
                    case 'c':
                        map["c"] = "";
                        arg = shift(arg, 1);
                        break;
                    case 'h':
                        map["h"] = "";
                        arg = shift(arg, 1);
                        break;
                    case 'j':
                        if (arg.length() == 1) {
                            waiting_to_eat = "j";
                        } else {
                            map["j"] = shift(arg, 1);
                        }
                        arg = "";
                        break;
                    default:
                        std::cerr << "Unknown option " << arg[0] << std::endl;
                        exit(1);
                }
            }

        } else {
            break;
        }
    }

    std::vector<std::string> the_rest;
    for (; i < argc; i++) {
        the_rest.push_back(argv[i]);
    }

    return {map, the_rest};
}

void
replace_all(
    std::string& source,
    const std::string& from,
    const std::string& to)
{
    std::string newString;
    newString.reserve(source.length());  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = source.find(from, lastPos)))
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

void print_help(std::string basename) {
    std::vector<std::string> message = {
        "{}, version 0.1",
        "Line oriented version of xargs",
        "",
        "usage: {} [-ch] [-j replstr] [utility [argument ...]]"
    };
    for (auto line : message) {
        replace_all(line, "{}", basename);
        std::cout << line << std::endl;
    }
}

void spawn(std::vector<std::string> argv) {
    std::stringstream ss;
    for (auto a : argv) {
        ss << a << ' ';
    }
    std::cout << ss.str() << std::endl;
    // system(ss.str().c_str());
}

int main(int argc, char** argv) {
    auto [args, rest] = parse_args(argc, argv);

    if (args.contains("h")) {
        print_help(argv[0]);
        return 0;
    }

    std::vector<std::string> inputs;
    for (std::string tmp;
        std::getline(std::cin, tmp);
        inputs.push_back('"' + tmp + '"')) {}

    std::string placeholder = args.contains("j") ? args["j"] : "%";

    if (args.contains("c")) {
        auto it = std::find(rest.begin(), rest.end(), placeholder);
        if (it != rest.end()) {
            rest.erase(it);
        }
        rest.insert(it, inputs.begin(), inputs.end());
        spawn(rest);
    } else {
        for (auto input : inputs) {
            auto copy = rest;
            for (auto& a : copy) {
                replace_all(a, placeholder, input);
            }
            spawn(copy);
        }
    }
    return 0;
}

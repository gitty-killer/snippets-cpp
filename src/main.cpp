#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static const std::vector<std::string> FIELDS = {"title", "language", "code"};
static const std::string NUMERIC_FIELD = null;
static const std::string STORE_PATH = "data/store.txt";

bool contains(const std::vector<std::string>& items, const std::string& v) {
  for (const auto& item : items) if (item == v) return true;
  return false;
}

std::map<std::string, std::string> parse_kv(const std::vector<std::string>& items) {
  std::map<std::string, std::string> record;
  for (const auto& item : items) {
    auto pos = item.find('=');
    if (pos == std::string::npos) throw std::runtime_error("Invalid item: " + item);
    auto key = item.substr(0, pos);
    auto value = item.substr(pos + 1);
    if (!contains(FIELDS, key)) throw std::runtime_error("Unknown field: " + key);
    if (value.find('|') != std::string::npos) throw std::runtime_error("Value may not contain '|' ");
    record[key] = value;
  }
  for (const auto& f : FIELDS) if (!record.count(f)) record[f] = "";
  return record;
}

std::string format_record(const std::map<std::string, std::string>& values) {
  std::ostringstream out;
  for (size_t i = 0; i < FIELDS.size(); i++) {
    const auto& k = FIELDS[i];
    out << k << '=' << values.at(k);
    if (i + 1 < FIELDS.size()) out << '|';
  }
  return out.str();
}

std::map<std::string, std::string> parse_line(const std::string& line) {
  std::map<std::string, std::string> values;
  std::stringstream ss(line);
  std::string part;
  while (std::getline(ss, part, '|')) {
    if (part.empty()) continue;
    auto pos = part.find('=');
    if (pos == std::string::npos) throw std::runtime_error("Bad part: " + part);
    values[part.substr(0, pos)] = part.substr(pos + 1);
  }
  return values;
}

std::vector<std::map<std::string, std::string>> load_records() {
  std::ifstream f(STORE_PATH);
  std::vector<std::map<std::string, std::string>> records;
  if (!f) return records;
  std::string line;
  while (std::getline(f, line)) {
    if (line.empty()) continue;
    records.push_back(parse_line(line));
  }
  return records;
}

void append_record(const std::map<std::string, std::string>& values) {
  std::ofstream f(STORE_PATH, std::ios::app);
  if (!f) throw std::runtime_error("Cannot open store");
  f << format_record(values) << "
";
}

std::string summary(const std::vector<std::map<std::string, std::string>>& records) {
  int count = (int)records.size();
  if (NUMERIC_FIELD.empty()) return "count=" + std::to_string(count);
  long total = 0;
  for (const auto& r : records) {
    auto it = r.find(NUMERIC_FIELD);
    if (it != r.end()) {
      total += std::stol(it->second);
    }
  }
  return "count=" + std::to_string(count) + ", " + NUMERIC_FIELD + "_total=" + std::to_string(total);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: init | add key=value... | list | summary
";
    return 2;
  }
  std::string cmd = argv[1];
  if (cmd == "init") {
    std::ofstream f(STORE_PATH); f.close();
    return 0;
  }
  if (cmd == "add") {
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) args.push_back(argv[i]);
    append_record(parse_kv(args));
    return 0;
  }
  if (cmd == "list") {
    for (const auto& r : load_records()) std::cout << format_record(r) << "
";
    return 0;
  }
  if (cmd == "summary") {
    std::cout << summary(load_records()) << "
";
    return 0;
  }
  std::cerr << "Unknown command: " << cmd << "
";
  return 2;
}

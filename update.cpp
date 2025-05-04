#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <sstream>
#include "../wyland-runtime/wylrt.hpp"
#include "../wyland-runtime/wnet.hpp"

using namespace wylma;

class Token {
public:
  std::string value;
  int         type;

  Token() : value(""), type(0) {}
  Token(const std::string &_value, int _type) : value(_value), type(_type) {}

  static const constexpr int INVALID  = -1;
  static const constexpr int MKDIR    = 0;
  static const constexpr int MKFILE   = 1;
  static const constexpr int DOWNLOAD = 2;
  static const constexpr int COMPARE  = 3;
  static const constexpr int PRINT    = 4;
  static const constexpr int SET      = 5;
  static const constexpr int READ     = 6;
  static const constexpr int IF       = 7;
  static const constexpr int ELSE     = 8;
  static const constexpr int ENDIF    = 9;
};

class UpdateLanguage {
private:
  wyland::wnet::HttpClient HttpClient;
  std::unordered_map<std::string, std::string> variables;
  std::vector<Token> tokens;

  std::string trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
  }

  std::string strip_quotes(const std::string &s) {
    size_t first = s.find_first_of('"');
    size_t last = s.find_last_of('"');
    if (first != std::string::npos && last != std::string::npos && last > first)
      return s.substr(first + 1, last - first - 1);
    return s;
  }

  std::string resolve(const std::string &s) {
    if (variables.count(s)) return variables[s];
    return s;
  }

  Token parse_line(const std::string &line) {
    std::string l = trim(line);
    if (l.empty()) return Token("", Token::INVALID);

    if (l.starts_with("MKDIR ")) {
      return Token(l.substr(6), Token::MKDIR);
    }
    if (l.starts_with("MKFILE ")) {
      return Token(l.substr(7), Token::MKFILE);
    }
    if (l.starts_with("DOWNLOAD ")) {
      auto pos = l.find(" TO ");
      if (pos == std::string::npos) return Token("", Token::INVALID);
      return Token(l.substr(9, pos - 9) + "=>" + l.substr(pos + 4), Token::DOWNLOAD);
    }
    if (l.starts_with("COMPARE ")) {
      auto pos = l.find("==");
      if (pos == std::string::npos) return Token("", Token::INVALID);
      std::string a = trim(l.substr(8, pos - 8));
      std::string b = trim(l.substr(pos + 2));
      return Token(a + "==" + b, Token::COMPARE);
    }
    if (l.starts_with("PRINT ")) {
      return Token(l.substr(6), Token::PRINT);
    }
    if (l.starts_with("READ ")) {
      return Token(l.substr(5), Token::READ);
    }
    if (l.starts_with("SET ")) {
      auto eq = l.find('=');
      if (eq == std::string::npos) return Token("", Token::INVALID);
      std::string name = trim(l.substr(4, eq - 4));
      std::string val = trim(l.substr(eq + 1));
      return Token(name + "=" + val, Token::SET);
    }
    if (l.starts_with("IF ")) {
      return Token(trim(l.substr(3)), Token::IF);
    }
    if (l == "ELSE") {
      return Token("", Token::ELSE);
    }
    if (l == "ENDIF") {
      return Token("", Token::ENDIF);
    }

    return Token("", Token::INVALID);
  }

  void create_directories(const std::string &dir) {
    std::filesystem::create_directories(dir);
  }

  void create_file(const std::string &file) {
    std::filesystem::create_directories(std::filesystem::path(file).parent_path());
    std::ofstream ofs(file);
  }

  void download(const std::string &url, const std::string &path) {
    std::cout << "Downloading " << path << " from " << url << ": " << std::flush;
    if (!HttpClient.DownloadToFile(url, path))
      std::cout << "[FAILED]" << std::endl;
    else
      std::cout << "[OK]" << std::endl;
  }

  void exec() {
    bool skip = false;
    bool in_if = false;
    bool else_block = false;

    for (size_t i = 0; i < tokens.size(); ++i) {
      const auto &token = tokens[i];

      if (token.type == Token::IF) {
        in_if = true;
        else_block = false;
        std::string cond = token.value;
        auto pos = cond.find("==");
        if (pos == std::string::npos) {
          skip = true;
          continue;
        }
        std::string left = resolve(trim(cond.substr(0, pos)));
        std::string right = strip_quotes(trim(cond.substr(pos + 2)));
        skip = (left != right);
        continue;
      }

      if (token.type == Token::ELSE) {
        if (in_if) {
          skip = !skip;
          else_block = true;
        }
        continue;
      }

      if (token.type == Token::ENDIF) {
        in_if = false;
        skip = false;
        continue;
      }

      if (skip) continue;

      switch (token.type) {
        case Token::MKDIR:
          create_directories(resolve(token.value));
          break;
        case Token::MKFILE:
          create_file(resolve(token.value));
          break;
        case Token::DOWNLOAD: {
          auto sep = token.value.find("=>");
          std::string url = resolve(token.value.substr(0, sep));
          std::string file = resolve(token.value.substr(sep + 2));
          download(url, file);
          break;
        }
        case Token::COMPARE: {
          auto sep = token.value.find("==");
          std::string a = resolve(trim(token.value.substr(0, sep)));
          std::string b = resolve(strip_quotes(trim(token.value.substr(sep + 2))));
          variables["FLAGS"] = (a == b) ? "true" : "false";
          break;
        }
        case Token::PRINT:
          std::cout << resolve(strip_quotes(token.value)) << std::endl;
          break;
        case Token::SET: {
          auto eq = token.value.find('=');
          std::string name = trim(token.value.substr(0, eq));
          std::string val = resolve(strip_quotes(trim(token.value.substr(eq + 1))));
          variables[name] = val;
          break;
        }
        case Token::READ: {
          std::string var = trim(token.value);
          std::string input;
          std::getline(std::cin, input);
          variables[var] = input;
          break;
        }
        default:
          break;
      }
    }
  }

public:
  int update(const std::string &path) {
    if (!std::filesystem::exists(path)) {
      std::cerr << "[e]: " << path << ": no such file or directory" << std::endl;
      return -1;
    }

    std::ifstream source(path);
    std::string segment;
    while (std::getline(source, segment, ';')) {
      Token tok = parse_line(segment);
      if (tok.type != Token::INVALID)
        tokens.push_back(tok);
    }
    source.close();

    exec();
    return 0;
  }
};

int main(int argc, char *const argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <script.txt>" << std::endl;
    return 1;
  }

  UpdateLanguage updater;
  return updater.update(argv[1]);
}

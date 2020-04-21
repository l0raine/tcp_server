#pragma once

// Add support for extra arguments later?
class commands {
  using func = std::function<void()>;
  std::unordered_map<std::string, std::any> m_cmds;

  std::mutex commands_mutex;

 public:
  void parse_input(const std::string &str) {
    auto it = m_cmds.find(str);
    if (it != m_cmds.end()) {
      std::any_cast<func>(it->second)();
    }
  }

  void add(const std::string &cmd, const func &cb) {
    std::lock_guard<std::mutex> lock(commands_mutex);

    m_cmds.insert({cmd, cb});
  }
};
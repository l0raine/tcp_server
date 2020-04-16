#pragma once

class client {
  int m_socket;
  std::string m_ip;

 public:
  client(){};
  client(int socket, const std::string_view &ip) : m_socket{socket}, m_ip{ip} {}
  bool send_message(const std::string &msg) {
    const size_t sent = send(m_socket, msg.data(), msg.size(), 0);
    return sent != msg.size();
  }
  int get_socket() const { return m_socket; }
  auto &get_ip() const { return m_ip; }
};
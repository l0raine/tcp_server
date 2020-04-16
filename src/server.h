#pragma once

namespace tcp {

struct message_data_t {
  std::string m_msg;
  client m_client;
};

struct client_data_t {
  // add what else?
  client m_client;
};

class server {
  int m_socket;

  std::vector<client> m_clients;  // internal client list

  event<client_data_t> connect_event;
  event<client_data_t> disconnect_event;
  event<message_data_t> receive_event;

  fd_set m_server_set;

 public:
  void start(const std::string &port) {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
      io::get()->error("failed to create server socket.");
      return;
    }
    struct addrinfo hints, *addrinfo = nullptr;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(nullptr, port.c_str(), &hints, &addrinfo);
    if (ret != 0) {
      io::get()->error("failed to get address info.");
      close(m_socket);
      return;
    }

    io::get()->info("binding port...");
    ret = bind(m_socket, addrinfo->ai_addr, addrinfo->ai_addrlen);
    if (ret < 0) {
      io::get()->error("failed to bind port.");
      freeaddrinfo(addrinfo);
      close(m_socket);
      return;
    }

    freeaddrinfo(addrinfo);

    ret = listen(m_socket, SOMAXCONN);
    if (ret < 0) {
      io::get()->error("failed to listen.");
      close(m_socket);
      freeaddrinfo(addrinfo);
      return;
    }
  }

  int get_socket() { return m_socket; }
  auto get_clients() { return m_clients; }
  auto &on_connect() { return connect_event; }
  auto &on_disconnect() { return disconnect_event; }
  auto &on_recv() { return receive_event; }

  bool ready() {
    FD_ZERO(&m_server_set);
    FD_SET(m_socket, &m_server_set);

    int maxfd = m_socket;
    for (const auto &c : m_clients) {
      int s = c.get_socket();
      FD_SET(s, &m_server_set);

      maxfd = std::max(s, maxfd);
    }

    // check for activity every 2.5s
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 5000;

    int ret = select(maxfd + 1, &m_server_set, nullptr, nullptr, &tv);
    if (ret < 0) {
      io::get()->error("select error : {}", strerror(errno));
      return false;
    }

    /*
        TO-DO: change return type maybe to check if a client timed out?, when
       there's no activity select returns 0 we could loop through clients and
       check against a timer to set a timeout.
    */

    return ret >= 0;
  }

  void accept_client() {
    if (!FD_ISSET(m_socket, &m_server_set)) return;

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int client_socket =
        accept(m_socket, reinterpret_cast<sockaddr *>(&addr), &len);
    auto ip = inet_ntoa(addr.sin_addr);
    if (client_socket < 0) {
      io::get()->warn("{} failed to accept.", ip);
      close(client_socket);
    } else {
      client cli(client_socket, ip);
      m_clients.push_back(cli);

      client_data_t data;
      data.m_client = cli;

      connect_event.call(data);
    }
  }

  void read() {
    std::array<char, 256> buffer;
    for (size_t i = 0; i < m_clients.size(); i++) {
      auto c = m_clients[i];
      int socket = c.get_socket();

      if (!FD_ISSET(socket, &m_server_set)) continue;

      buffer.fill(0);
      const int read = recv(socket, &buffer[0], buffer.size(), 0);
      if (read > 0) {
        std::string msg{buffer.data()};
        message_data_t msg_data;
        msg_data.m_msg = msg;
        msg_data.m_client = c;

        receive_event.call(msg_data);
      } else {
        client_data_t cli_data;
        cli_data.m_client = c;

        disconnect_event.call(cli_data);

        m_clients.erase(m_clients.begin() + i);
      }
    }
  }

  void main_loop() {
    for (;;) {
      if (!ready()) break;

      accept_client();

      read();
    }
  }
};

}  // namespace tcp
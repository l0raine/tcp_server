#include "include.h"
#include "io.h"
#include "events.h"
#include "client.h"
#include "server.h"

int main(int argc, char* argv[]) {
  io::init();

  std::vector<std::string> cmdline(argv + 1, argv + argc);
  std::unordered_map<std::string, std::optional<std::string>> args;
  for (auto it = cmdline.begin(); it != cmdline.end(); ++it) {
    auto next = std::next(it);
    if (next == cmdline.end()) {
      args.insert({*it, std::nullopt});
      continue;
    }

    args.insert({*it, *next});
    cmdline.erase(next);
  }

  auto port = args["-p"].value_or("6666");
  io::get()->info("starting local server on port {}", port);

  tcp::server server;
  server.start(port);

  io::get()->info("server listening for new connections.");

  server.on_connect().add([&](tcp::client_data_t data) {
    io::get()->info("client {} connected.", data.m_client.get_ip());
  });

  server.on_recv().add([&](tcp::message_data_t data) {
    // io::get()->info(data.m_msg);
    auto socket = data.m_client.get_socket();
    for (auto& c : server.get_clients()) {
      if (c.get_socket() != socket) {
        c.send_message(data.m_msg);
      }
    }
  });

  server.on_disconnect().add([&](tcp::client_data_t data) {
    io::get()->info("{} disconnected.", data.m_client.get_ip());
  });

  server.main_loop();
}
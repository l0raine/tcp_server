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
  if(!server.start(port))
    return 0;

  io::get()->info("server listening for new connections.");

  server.on_connect().add([&](tcp::client_data_t &data) {
    io::get()->info("client {} connected.", data.m_client.get_ip());
  });

  server.on_recv().add([&](tcp::message_data_t &sender) {
    // io::get()->info(sender.m_msg);
    auto socket = sender.m_client.get_socket();
    auto ip = sender.m_client.get_ip();
    for (auto& c : server.get_clients()) {
      if (c.get_socket() != socket) {
        auto msg = fmt::format("[{}] {}", ip, sender.m_msg);
        if(!c.send_message(msg))
          io::get()->warn("failed to send message from {} to {}.", ip, c.get_ip());
      }
    }
  });

  server.on_disconnect().add([&](tcp::client_data_t &data) {
    io::get()->info("{} disconnected.", data.m_client.get_ip());
  });

  server.run();
}
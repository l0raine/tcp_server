#include "include.h"
#include "io.h"
#include "events.h"
#include "client.h"
#include "server.h"
#include "commands.h"

int main(int argc, char *argv[]) {
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
  if (server.start(port)) {
    io::get()->info("server listening for new connections.");
  }

  server.on_connect().add([&](tcp::client_data_t &data) {
    io::get()->info("client {} connected.", data.m_client.get_ip());
  });

  server.on_recv().add([&](tcp::message_data_t &sender) {
    // io::get()->info(sender.m_msg);
    auto socket = sender.m_client.get_socket();
    auto ip = sender.m_client.get_ip();
    for (auto &c : server.get_clients()) {
      if (c.get_socket() != socket) {
        auto msg = fmt::format("[{}] {}", ip, sender.m_msg);
        if (!c.send_message(msg))
          io::get()->warn("failed to send message from {} to {}.", ip,
                          c.get_ip());
      }
    }
  });

  server.on_disconnect().add([&](tcp::client_data_t &data) {
    close(data.m_client.get_socket());
    io::get()->info("{} disconnected.", data.m_client.get_ip());
  });

  commands cmds;

  cmds.add("clients", [&]() {
    io::get()->info("clients connected {}", server.get_clients().size());
  });

  cmds.add("stop", [&]() { server.set_running(false); });

  auto input_thread = [&]() {
    while (server.is_running()) {
      std::string cmd;
      getline(std::cin, cmd);
      cmds.parse_input(cmd);
    }
  };

  // move this out of the lambda
  auto main_thread = [&]() {
    while (server.is_running()) {
      if (!server.ready()) break;  // maybe not break here?

      server.accept_client();

      server.read();
    }
  };

  std::thread t{main_thread};
  std::thread t1{input_thread};
  
  if (t.joinable()) {
    t1.join();
  }
  t.join();

  server.shutdown();
}
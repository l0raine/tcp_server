#include "include.h"
#include "io.h"
#include "callbacks.h"

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


  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == -1){
  	io::get()->error("failed to create server socket.");
  	abort();
  }

  struct addrinfo hints, *addrinfo = nullptr;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  int ret = getaddrinfo(nullptr, port.c_str(),&hints, &addrinfo);
  if(ret != 0){
  	io::get()->error("failed to get address info.");
  	close(server_socket);
  	exit(0);
  }

  io::get()->info("binding port...");
  ret = bind(server_socket, addrinfo->ai_addr, addrinfo->ai_addrlen);
  if(ret < 0){
  	io::get()->error("failed to bind port.");
  	close(server_socket);
  	exit(0);
  }

  ret = listen(server_socket, SOMAXCONN);
  if (ret < 0) {
    io::get()->error("failed to listen.");
  	close(server_socket);
  	exit(0);
  }

  io::get()->info("server listening for new connections.");

  std::thread t{callbacks::server_loop, server_socket};
  t.join();

}
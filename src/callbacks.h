#pragma once

using client = std::pair<int, std::string>;

namespace callbacks {
	void server_loop(int server_socket){
		std::vector<client> clients;

		fd_set rfds;
		for(;;){
			FD_ZERO(&rfds);
			FD_SET(server_socket, &rfds);

			int maxfd = server_socket;
			for(auto &c : clients){
				FD_SET(c.first, &rfds);

				maxfd = std::max(maxfd, c.first);
			}

			// check for activity every 2.5s
			struct timeval tv;
		    tv.tv_sec = 2;
		    tv.tv_usec = 5000;

		    int ret = select(maxfd + 1, &rfds, nullptr, nullptr, &tv);
		    if(ret < 0){
		    	io::get()->error("select error : {}", strerror(errno));
		    	continue;
		    }

		    // no activity
		    if(ret == 0){
		    	continue;
		    }

		    if(FD_ISSET(server_socket, &rfds)){
		    	struct sockaddr_in addr;
			    socklen_t len = sizeof(addr);
			    int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&addr), &len);
			    auto ip = inet_ntoa(addr.sin_addr);
			    if(client_socket == -1){
			    	io::get()->warn("{} failed to accept.", ip);
			    	close(client_socket);
			    }
			    else {
			    	clients.emplace_back(std::make_pair(client_socket, ip));
			    	io::get()->info("{} connected.", ip);
			    }
		    }

		    std::array<char, 256> buffer;
		    for(size_t i = 0; i < clients.size(); i++){
		    	auto client = clients[i];
		    	int client_sock = client.first;
		    	auto client_ip = client.second;

		    	if(!FD_ISSET(client_sock, &rfds)){
		    		continue;
		    	}

		    	buffer.fill(0);
		    	const int read_bytes = recv(client_sock, &buffer[0], buffer.size(), 0);
		    	if(read_bytes > 0){
		    		int sender = client_sock;
		    		auto buf = fmt::format("[{}] {}", client_ip, buffer.data());

		    		for(auto[s, ip] : clients){
		    			if(s != sender){
		    				send(s, buf.data(), buf.size(), 0);
		    			}
		    		}
		    	}
		    	else {
		    		io::get()->info("{} disconnected.", client_ip);
		    		close(client_sock);
		    		clients.erase(clients.begin()+i);
		    	}
		    }
		}
	}
}
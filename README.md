## Preface
A simple TCP non blocking server that relays messages a client sends to other clients,
something like a chat server,this can be adapted to do anything in any use case.
The server only works on linux, I'll be accepting pull requests if anyone wants to improve the code or port it to windows.

## How to build
```
mkdir build
cd build
cmake ..
make
```

## Contributing
Please make sure you format your code using clang with the Google style before your contribute.

## Credits 
- [spdlog](https://github.com/gabime/spdlog)

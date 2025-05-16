#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <atomic>

using boost::asio::ip::tcp;
std::atomic<bool> isConnected(false);

void ReceiveMessages(tcp::socket& socket) {
    try {
        char buffer[512];
        while (isConnected) {
            std::memset(buffer, 0, sizeof(buffer));
            size_t length = socket.read_some(boost::asio::buffer(buffer));
            if (length > 0) {
                std::cout << "[Server]: " << std::string(buffer, length) << std::endl;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Connection lost: " << e.what() << std::endl;
        isConnected = false;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "7777");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        isConnected = true;

        std::cout << "✅ Connected to the server\n";

        // Send login first
        std::cout << "Enter username: ";
        std::string user;
        std::getline(std::cin, user);

        std::cout << "Enter password: ";
        std::string pass;
        std::getline(std::cin, pass);

        std::string loginMsg = "login:" + user + ":" + pass;
        boost::asio::write(socket, boost::asio::buffer(loginMsg));

        // Start receiving thread
        std::thread receiver(ReceiveMessages, std::ref(socket));

        std::string message;
        while (isConnected) {
            std::getline(std::cin, message);

            if (message == "exit") {
                isConnected = false;
                socket.close();
                break;
            }

            if (!message.empty()) {
                boost::asio::write(socket, boost::asio::buffer(message));
            }
        }

        receiver.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    std::cout << "Client shutdown.\n";
    return 0;
}
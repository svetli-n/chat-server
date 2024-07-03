#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>

using boost::asio::ip::tcp;

class ChatServer {
public:
    ChatServer(boost::asio::io_context& io_context, int port)
            : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }

private:
    void start_accept() {
        auto new_session = std::make_shared<tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*new_session, [this, new_session](boost::system::error_code ec) {
            if (!ec) {
                handle_new_connection(new_session);
            }
            start_accept();
        });
    }

    void handle_new_connection(const std::shared_ptr<tcp::socket>& socket) {
        // Add socket to the list of connected clients
        clients_.push_back(socket);

        // Start reading messages from the client
        read_message(socket);
    }

    void read_message(const std::shared_ptr<tcp::socket>& socket) {
        auto buffer = std::make_shared<std::vector<char>>(1024);
        socket->async_read_some(boost::asio::buffer(*buffer),
                                [this, socket, buffer](boost::system::error_code ec, std::size_t length) {
                                    if (!ec) {
                                        // Broadcast the message to all connected clients
                                        broadcast_message(buffer->data(), length, socket);

                                        // Continue reading messages from the client
                                        read_message(socket);
                                    } else {
                                        // Handle disconnection
                                        clients_.erase(std::remove(clients_.begin(), clients_.end(), socket), clients_.end());
                                    }
                                });
    }

    void broadcast_message(const char* message, std::size_t length, const std::shared_ptr<tcp::socket>& sender) {
        for (auto& client : clients_) {
            if (client != sender) {
                boost::asio::async_write(*client, boost::asio::buffer(message, length),
                                         [](boost::system::error_code /*ec*/, std::size_t /*length*/) {
                                             // Handle write completion (optional)
                                         });
            }
        }
    }


    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<tcp::socket>> clients_;
};

int main() {
//    Boost version: 1.85.0
//    Boost.Asio version: 1.30.2
    try {
        boost::asio::io_context io_context;
        ChatServer server(io_context, 12345);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

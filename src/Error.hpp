#ifndef ERROR_HPP_
#define ERROR_HPP_
#include <exception>
#include <string>

class Error : public std::exception {
    private:
    std::string message;
    public:
    Error(const std::string &err) : message(err){};
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class ParserError : public Error {
    public:
        explicit ParserError(std::string error) : Error(std::move(error)) {}
};

class DLLoaderError : public Error {
    public:
        explicit DLLoaderError(std::string error) : Error(std::move(error)) {}
};

#endif /* !ERROR_HPP_ */

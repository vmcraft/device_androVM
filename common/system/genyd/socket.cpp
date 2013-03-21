
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

#include "socket.hpp"

Socket::Socket(int socket)
  : socket(socket)
{

}

Socket::~Socket(void)
{
  if (socket != -1) {
    close(socket);
  }
}

Socket::ReadStatus Socket::read(void)
{
  int len = 0;
  char *cmd = 0;
  char buffer[1024];

  if ((len = recv(socket, buffer, 1023, MSG_NOSIGNAL)) < 0) {
    SLOGE("recv() error");
  }

  if (len <= 0) {
    return Socket::ReadError;
  }

  istream.write(buffer, len);

  if (request.ParseFromIstream(&istream)) {
    return (Socket::NewMessage);
  } else {
    return Socket::NoMessage;
  }
}

bool Socket::hasReplies(void) const
{
  return replies.size();
}

Socket::WriteStatus Socket::reply(void)
{
  std::string data;

  Reply *reply = replies.front();
  replies.pop();

  if (!reply->SerializeToString(&data)) {
    ALOGE("Can't serialize reply");
    delete reply;
    return Socket::BadSerialize;
  }
  if (send(socket, data.c_str(), data.size(), MSG_NOSIGNAL) < 0) {
    ALOGE("Can't send reply");
    delete reply;
    return Socket::WriteError;
  }
  delete reply;
  return Socket::WriteSuccess;
}

int Socket::getFD(void) const
{
  return socket;
}

const Request &Socket::getRequest(void) const
{
  return request;
}

void Socket::addReply(Reply *reply)
{
  replies.push(reply);
}
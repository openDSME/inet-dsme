#include "RPCallable.h"
#include <thread>
#include "Topic.h"

class DemoServer : public RPCallable<DemoServer>
{
public:
  DemoServer();
  ~DemoServer();
  int adding(int a, int b);
  void handleEvent1(int a);

private:
  void eventLoop();
  std::thread eventThread;
  bool running;

  Topic<int> topic;
};

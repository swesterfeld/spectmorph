// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smsignal.hh"

using namespace SpectMorph;

using std::string;

Signal<void()> simple_signal;

struct Receiver : SignalReceiver
{
  string message;
  Receiver (const string& message) :
    message (message)
  {
    connect (simple_signal, std::function<void()> ([&]() { simple_slot(); }));
  }
  void
  simple_slot()
  {
    printf ("%s\n", message.c_str());
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  printf ("---- expect: nothing\n");
  simple_signal();
  printf ("---- end ----\n\n");

  Receiver *receiver = new Receiver ("receiver 1");
  printf ("---- expect: something\n");
  simple_signal();
  printf ("---- end ----\n\n");

  delete receiver;
  printf ("---- expect: nothing\n");
  simple_signal();
  printf ("---- end ----\n\n");
}
